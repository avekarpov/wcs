#ifndef WCS_ORDER_CONTROLLER_HPP
#define WCS_ORDER_CONTROLLER_HPP

#include <list>
#include <map>
#include <set>
#include <variant>

#include "events/cancel_order.hpp"
#include "events/fill_order.hpp"
#include "events/order_update.hpp"
#include "events/place_order.hpp"
#include "utilits/exception.hpp"
#include "utilits/side_comparison.hpp"

namespace wcs
{

template <class Consumer>
class OrderController
{
private:
    template <class Order_t, class Derived>
    class UnsafeIndexedOrderList
    {
    private:
        using OrderList = std::list<OrderHandler<Order_t>>;
        using ListIndex = std::map<OrderId, typename OrderList::iterator>;
    
    public:
        using SharedOrderList = std::shared_ptr<const OrderList>;
        
    public:
        UnsafeIndexedOrderList() : _list { std::make_shared<OrderList>() } { }
        
        SharedOrderList get() const
        {
            return _list;
        }
    
        template <class ... Args>
        void add(OrderId id, Args &&...args)
        {
            static_cast<Derived &>(*this).addImpl(id, std::forward<Args>(args)...);
        }
        
        void remove(OrderId id)
        {
            auto it = _index.find(id);
            
            _list->erase(it->second);
            _index.erase(it);
        }
    protected:
        std::shared_ptr<OrderList> _list;
        ListIndex _index;
        
    };
    
    template <Side S>
    class MarketOrders : public UnsafeIndexedOrderList<MarketOrder<S>, MarketOrders<S>>
    {
    private:
        using Base = UnsafeIndexedOrderList<MarketOrder<S>, MarketOrders<S>>;
        friend Base;

    private:
        void addImpl(OrderId id, Amount amount)
        {
            _list->emplace_back(id, amount);
            _index.emplace(id, --_list->end());
        }
        
    private:
        using Base::_list;
        using Base::_index;
        
    };
    
    template <Side S>
    class LimitOrders : public UnsafeIndexedOrderList<LimitOrder<S>, LimitOrders<S>>
    {
    private:
        using Base = UnsafeIndexedOrderList<LimitOrder<S>, LimitOrders<S>>;
        friend Base;
        
    private:
        void addImpl(OrderId id, Amount amount, Price price)
        {
            for (auto it = _list->begin(); it != _list->end(); ++it) {
                if (utilits::sideLess<S>(it->price(), price)) {
                    _index.emplace(id, _list->emplace(it, id, amount, price));
                    
                    return;
                }
            }
            
            _list->emplace_back(id, amount, price);
            _index.emplace(id, --_list->end());
        }
        
    private:
        using Base::_list;
        using Base::_index;
        
    };
    
    template <Side S, OrderType OT>
    struct OrderSideType
    {
    
    };
    
public:
    template <Side S>
    using MarketOrderList = typename MarketOrders<S>::SharedOrderList;
    
    template <Side S>
    using LimitOrderList = typename LimitOrders<S>::SharedOrderList;
    
public:
    void setConsumer(std::shared_ptr<Consumer> consumer)
    {
        _consumer = consumer;
    }
    
    template <Side S, OrderType OT>
    void process(const events::PlaceOrder<S, OT> &event)
    {
        // TODO: check order params
        
        if (auto it = _order_table.find(event.client_order_id); it != _order_table.end()) {
            throw WCS_EXCEPTION(std::runtime_error, "Duplicate order id");
        }
        
        if constexpr (OT == OrderType::Market) {
            _market_orders.template get<S>().add(event.client_order_id, event.amount);
        }
        else if constexpr (OT == OrderType::Limit) {
            _limit_orders.template get<S>().add(event.client_order_id, event.amount, event.price);
        }
        else {
            static_assert(OT == OrderType::Market || OT == OrderType::Limit);
        }
    
        _order_table.emplace(event.client_order_id, OrderSideType<S, OT>{ });
        
        _consumer.lock()->process(events::OrderUpdate<OrderStatus::New>
        {
            event.client_order_id
        });
    }
    
    void process(const events::CancelOrder &event)
    {
        if (auto it = _order_table.find(event.client_order_id); it == _order_table.end()) {
            throw WCS_EXCEPTION(std::runtime_error, "Invalid order id");
        }
        
        std::visit([&]<Side S, OrderType OT>(OrderSideType<S, OT>)
        {
            if constexpr (OT == OrderType::Market) {
                _market_orders.template get<S>().remove(event.client_order_id);
            }
            else if constexpr (OT == OrderType::Limit) {
                _limit_orders.template get<S>().remove(event.client_order_id);
            }
            else {
                static_assert(OT == OrderType::Market || OT == OrderType::Limit);
            }
    
            _consumer.lock()->process(events::OrderUpdate<OrderStatus::Canceled>
            {
                event.client_order_id
            });
        },
        _order_table.find(event.client_order_id)->second);
    }
    
    SidePair<MarketOrderList> marketOrders() const
    {
        return { _market_orders.template get<Side::Buy>().get(), _market_orders.template get<Side::Sell>().get() };
    }

    SidePair<LimitOrderList> limitOrders() const
    {
        return { _limit_orders.template get<Side::Buy>().get(), _limit_orders.template get<Side::Sell>().get() };
    }
    
private:
    std::weak_ptr<Consumer> _consumer;
    
    std::map<
        OrderId,
        std::variant<
            OrderSideType<Side::Buy, OrderType::Market>,
            OrderSideType<Side::Sell, OrderType::Market>,
            OrderSideType<Side::Buy, OrderType::Limit>,
            OrderSideType<Side::Sell, OrderType::Limit>>> _order_table;
    
    SidePair<MarketOrders> _market_orders;
    SidePair<LimitOrders> _limit_orders;
    
};

} // namespace wcs

#endif //WCS_ORDER_CONTROLLER_HPP
