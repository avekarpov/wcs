#ifndef WCS_MATCHING_ENGINE_HPP
#define WCS_MATCHING_ENGINE_HPP

#include "events/decrease_level.hpp"
#include "events/move_order_to.hpp"
#include "events/shift_order.hpp"
#include "events/fill_order.hpp"
#include "events/trade.hpp"
#include "logger.hpp"
#include "order_manager.hpp"

namespace wcs
{

class MatchingEngineLogger
{
protected:
    inline static Logger _logger { "MatchingEngine" };
};

template <class Consumer_t>
class MatchingEngine : public MatchingEngineLogger
{
public:
    void setConsumer(std::shared_ptr<Consumer_t> consumer)
    {
        _consumer = consumer;
    }
    
    void setOrderManager(const std::shared_ptr<const OrderManager> &order_manager)
    {
        _order_manager = order_manager;
    }
    
    void process(const events::Trade &event)
    {
        _logger.gotEvent(event);
    
        executeMarketOrders(event);
        executeLimitOrders(event);
    }
    
private:
    void executeMarketOrders(const events::Trade &event)
    {
        if (event.maker_side == Side::Buy) {
            executeMarketOrders<Side::Sell>(event.price, event.volume);
        }
        else {
            executeMarketOrders<Side::Buy>(event.price, event.volume);
        }
    }
    
    template <Side S>
    void executeMarketOrders(const Price &price, const Amount &volume)
    {
        auto rest_volume = volume;
        std::map<OrderId, Amount> _orders_fill_volume;
        
        const auto market_orders = _order_manager.lock()->marketOrders().get<S>();
        
        for (const auto &order : *market_orders) {
            const auto fill_for = std::min(rest_volume, order.restAmount());
            
            _orders_fill_volume.emplace(order.id(), fill_for);
            _logger.debug(R"(Order: {}, matched for {} volume)", order, fill_for);
            
            rest_volume -= fill_for;
            
            if (!rest_volume) {
                break;
            }
        }
        
        for (const auto &order_fill_volume : _orders_fill_volume) {
            generateFillOrder(order_fill_volume.first, order_fill_volume.second);
        }
    }
    
    void executeLimitOrders(const events::Trade &event)
    {
        if (event.maker_side == Side::Buy) {
            executeLimitOrders<Side::Buy>(event.price, event.volume);
        }
        else {
            executeLimitOrders<Side::Sell>(event.price, event.volume);
        }
    }
    
    template <Side S>
    void executeLimitOrders(const Price &price, const Amount &volume)
    {
        std::map<OrderId, Amount> _orders_fill_volume;
        
        const auto limit_orders = _order_manager.lock()->limitOrders().get<S>();
        auto order = limit_orders->begin();
    
        while (order != limit_orders->end() && utilits::sideLess<S>(price, order->price())) {
            ++order;
        }
        
        auto rest_volume_for_fill = volume;
        Amount spend_volume_for_shift { 0 };
        while (order != limit_orders->end() && price == order->price() && order->volumeBefore() < volume) {
            if (!rest_volume_for_fill) {
                break;
            }
            
            // spend rest volume for shift order
            if (order->volumeBefore()) {
                rest_volume_for_fill -= order->volumeBefore();
                spend_volume_for_shift += order->volumeBefore();
                generateMoveOrderTo(order->id(), Amount { 0 });
            }
    
            const auto fill_for = std::min(rest_volume_for_fill, order->restAmount());
            _orders_fill_volume.emplace(order->id(), fill_for);
            _logger.debug(R"(Order: {}, matched for {} volume)", *order, fill_for);
    
            rest_volume_for_fill -= fill_for;
            
            ++order;
        }
        
        const auto decrease_level_volume = spend_volume_for_shift + rest_volume_for_fill;
        if (decrease_level_volume) {
            generateDecreaseLevel<S>(price, decrease_level_volume);
            
            // TODO: move this logic in order book maybe
            while (order != limit_orders->end() && price == order->price()) {
                generateShiftOrder(order->id(), decrease_level_volume);
                ++order;
            }
        }
        
        for (const auto &order_fill_volume : _orders_fill_volume) {
            generateFillOrder(order_fill_volume.first, order_fill_volume.second);
        }
    }
    
private:
    void generateShiftOrder(OrderId order_id, const Amount &volume) const
    {
        _consumer.lock()->process(events::ShiftOrder
        {
            Ts { 0 }, EventId { 0 }, // TODO: change for event builder
            order_id,
            volume
        });
    }
    
    void generateMoveOrderTo(OrderId order_id, const Amount &volume_before) const
    {
        _consumer.lock()->process(events::MoveOrderTo
        {
            Ts { 0 }, EventId { 0 }, // TODO: change for event builder
            order_id,
            volume_before
        });
    }
    
    void generateFillOrder(OrderId order_id, const Amount &amount)
    {
        _consumer.lock()->process(events::FillOrder
        {
            Ts { 0 }, EventId { 0 }, // TODO: change for event builder
            order_id,
            amount
        });
    }
    
    template <Side S>
    void generateDecreaseLevel(const Price &price, const Amount &volume)
    {
        _consumer.lock()->process(events::DecreaseLevel<S>
        {
            Ts { 0 }, EventId { 0 }, // TODO: change for event builder
            price,
            volume
        });
    }
    
private:
    std::weak_ptr<Consumer_t> _consumer;
    std::weak_ptr<const OrderManager> _order_manager;
    
};

} // namespace wcs

#endif //WCS_MATCHING_ENGINE_HPP
