#ifndef WCS_ORDERBOOK_HPP
#define WCS_ORDERBOOK_HPP

#include <vector>
#include <iostream>

#include "entities/level.hpp"
#include "entities/side.hpp"
#include "events/move_order.hpp"
#include "events/order_update.hpp"
#include "events/orderbook_update.hpp"
#include "order_manager.hpp"
#include "utilits/side_comparison.hpp"

namespace wcs
{

template <class Consumer>
class Orderbook
{
public:
    
public:
    void setConsumer(std::shared_ptr<Consumer> consumer)
    {
        _consumer = consumer;
    }
    
    void setOrderManager(const std::shared_ptr<const OrderManager> &order_manager)
    {
        _order_manager = order_manager;
    }
    
    void processAndComplete(events::OrderbookUpdate &event)
    {
        // TODO: update all orders volume before
    
        _historical_depth = event.depth;
        
        completeDepth();
        
        event.depth = _completed_depth;
    }
    
    void process(const events::OrderUpdate<OrderStatus::New> &event)
    {
        const auto &orderHandler = _order_manager.lock()->get(event.client_order_id);
        
        const std::optional<Amount> volume_before = orderHandler.side() == Side::Buy
            ? getLevelVolume<Side::Buy>(orderHandler.price())
            : getLevelVolume<Side::Sell>(orderHandler.price());
        
        if (volume_before) {
            generateOrderMove(event.client_order_id, volume_before.value());
        }
        else {
            generateOrderMove(event.client_order_id, Amount { std::numeric_limits<double>::max() });
        }
    }
    
    template <Side S>
    std::optional<Amount> getLevelVolume(const Price &price) const
    {
        const auto &depth = _historical_depth.template get<S>();
        for (const auto &level : depth) {
            if (utilits::sideGreaterEqual<S>(price, level.price())) {
                if (price == level.price()) {
                    return level.volume();
                }
                
                return Amount { 0 };
            }
        }
        
        return std::nullopt;
    }
    
private:
    void completeDepth()
    {
        completeSideDepth<Side::Buy>();
        completeSideDepth<Side::Sell>();
    }
    
    template <Side S>
    void completeSideDepth()
    {
        auto &depth = _completed_depth.template get<S>();
        const auto strategy_orders = _order_manager.lock()->limitOrders().get<S>();
       
        depth = _historical_depth.template get<S>();
        
        auto order = strategy_orders->begin();
        auto level = depth.begin();
        while (level != depth.end() && order != strategy_orders->end()) {
            if (utilits::sideLessEqual<S>(level->price(), order->price())) {
                if (level->price() == order->price()) {
                    level->updateVolume(level->volume() + order->amount() - order->filledAmount());
                }
                else {
                    level = ++depth.insert(level, Level<S> { order->price(), order->amount() - order->filledAmount() });
                }

                ++order;

                continue;
            }

            ++level;
        }
    }
    
    void generateOrderMove(OrderId order_id, Amount volume_before)
    {
        _consumer.lock()->process(events::MoveOrder
        {
            Ts { 0 }, EventId { 0 }, // TODO: change for event builder
            order_id,
            volume_before
        });
    }
    
private:
    std::weak_ptr<Consumer> _consumer;
    std::weak_ptr<const OrderManager> _order_manager;
    
    SidePair<Depth> _historical_depth;
    SidePair<Depth> _completed_depth;

};

} // namespace wcs

#endif //WCS_ORDERBOOK_HPP
