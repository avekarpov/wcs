#ifndef WCS_ORDER_MANAGER_HPP
#define WCS_ORDER_MANAGER_HPP

#include <list>
#include <map>
#include <variant>
#include <cassert>

#include "entities/order.hpp"
#include "utilits/side_comparison.hpp"
#include "utilits/exception.hpp"

namespace wcs
{

using OrderList = std::list<OrderHandler>;
using SharedOrderList = std::shared_ptr<const OrderList>;

class UnsafeIndexedOrderList
{
private:
    using Index = typename OrderList::iterator;
    using ListIndex = std::map<OrderId, Index>;

public:
    UnsafeIndexedOrderList();
    
    OrderHandler &get(OrderId id);
    
    const OrderHandler &get(OrderId id) const;
    
    void remove(OrderId id);
    
    std::shared_ptr<const OrderList> list() const;
    
protected:
    std::shared_ptr<OrderList> _list;
    ListIndex _index;
    
};

template <Side S>
class MarketUnsafeIndexedOrderList : public UnsafeIndexedOrderList
{
public:
    void add(OrderId id, const Amount &amount)
    {
        _list->emplace_back(id, S, OrderType::Market, amount);
        _index.emplace(id, --_list->end());
    }
    
};

template <Side S>
class LimitUnsafeIndexedOrderList : public UnsafeIndexedOrderList
{
public:
    void add(OrderId id, const Amount &amount, const Price &price)
    {
        for (auto it = _list->begin(); it != _list->end(); ++it) {
            if (utilits::sideLess<S>(it->price(), price)) {
                _index.emplace(id, _list->emplace(it, id, S, OrderType::Limit, amount, price));
            
                return;
            }
        }
    
        _list->emplace_back(id, S, OrderType::Limit, amount, price);
        _index.emplace(id, --_list->end());
    }
    
};

class OrderManager
{
private:
    enum class OrderVariant
    {
        MarketBuy,
        MarketSell,
        LimitBuy,
        LimitSell
    };
    
    template <Side S, OrderType OT>
    constexpr OrderVariant toOrderVariant()
    {
        if constexpr (OT == OrderType::Market) {
            if constexpr (S == Side::Buy) {
                return OrderVariant::MarketBuy;
            }
            else {
                return OrderVariant::MarketSell;
            }
        }
        else {
            if constexpr (S == Side::Buy) {
                return OrderVariant::LimitBuy;
            }
            else {
                return OrderVariant::LimitSell;
            }
        }
    }
    
    using OrderTable = std::map<OrderId, OrderVariant>;
    
public:
    template <Side S>
    using SideSharedOrderList = SharedOrderList;
    
public:
    template <Side S, OrderType OT, class ...Args>
    void add(OrderId id, Args &&...args)
    {
        auto it = _order_table.find(id);
    
        if (it != _order_table.end()) {
            throw WCS_EXCEPTION(std::runtime_error, "Duplicate order id");
        }
        
        if constexpr (OT == OrderType::Market) {
            _market_orders.get<S>().add(id, std::forward<Args>(args)...);
        }
        else {
            _limit_orders.get<S>().add(id, std::forward<Args>(args)...);
        }
        
        _order_table.emplace(id, toOrderVariant<S, OT>());
    }
    
    OrderHandler &get(OrderId id);
    
    const OrderHandler &get(OrderId id) const;
    
    void remove(OrderId id);
    
    SidePair<SideSharedOrderList> marketOrders() const;
    
    SidePair<SideSharedOrderList> limitOrders() const;
    
private:
    OrderTable _order_table;

    SidePair<MarketUnsafeIndexedOrderList> _market_orders;
    SidePair<LimitUnsafeIndexedOrderList> _limit_orders;
    
};

} // namespace wcs

#endif //WCS_ORDER_MANAGER_HPP
