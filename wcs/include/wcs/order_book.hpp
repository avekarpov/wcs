#ifndef WCS_ORDERBOOK_HPP
#define WCS_ORDERBOOK_HPP

#include <iostream>
#include <vector>

#include "entities/level.hpp"
#include "entities/side.hpp"
#include "event_builder.hpp"
#include "events/decrease_level.hpp"
#include "events/freeze_order.hpp"
#include "events/move_order_to.hpp"
#include "events/order_book_update.hpp"
#include "events/order_update.hpp"
#include "logger.hpp"
#include "order_manager.hpp"
#include "time_manager.hpp"
#include "utilits/side_comparison.hpp"

namespace wcs
{

class OrderBookLogger
{
protected:
    inline static Logger _logger { "OrderBook" };
    
};

template <class Consumer_t>
class OrderBook : public OrderBookLogger
{
public:
    void setConsumer(const std::shared_ptr<Consumer_t> &consumer)
    {
        _consumer = consumer;
    }
    
    void setOrderManager(const std::shared_ptr<const OrderManager> &order_manager)
    {
        _order_manager = order_manager;
    }
    
    void processAndComplete(events::OrderBookUpdate &event)
    {
        _logger.gotEvent(event);
    
        assert([&] ()
        {
            const auto is_depth_correct = [&] <Side S> (const Depth<S> &depth)
            {
                for (auto it = depth.begin(), prev = it++; it != depth.end(); ++it) {
                    if (!utilits::sideLess<S>(it->price(), prev->price())) {
                        return false;
                    }
                }
                
                return true;
            };
            
            return
                is_depth_correct(event.depth.get<Side::Buy>()) &&
                is_depth_correct(event.depth.get<Side::Sell>());
        }
        ());
        
        moveOrders(event);
        updateHistoricalDepth(event);
        updateFreezedOrders();
        completeDepth(event);
    
        assert([&] ()
        {
            const auto is_freezing_correct = [] (auto strategy_orders)
            {
                auto order = strategy_orders->begin();
                while (order != strategy_orders->end()) {
                    if (order->isFreezed()) {
                        break;
                    }
                    
                    ++order;
                }
    
                while (order != strategy_orders->end()) {
                    if (!order->isFreezed()) {
                        return false;
                    }
                    
                    ++order;
                }
                
                return true;
            };
            
            const auto strategy_orders = _order_manager.lock()->limitOrders();
            
            return
                is_freezing_correct(strategy_orders.get<Side::Buy>()) &&
                is_freezing_correct(strategy_orders.get<Side::Sell>());
        }
        ());
    
        assert([&] ()
        {
            const auto is_rest_amount_correct = [] (auto strategy_orders)
            {
                if (strategy_orders->empty()) {
                    return true;
                }
                
                auto order = strategy_orders->begin();
                if (!order->restAmount()) {
                    return false;
                }
                ++order;
    
                while (order != strategy_orders->end()) {
                    if (order->restAmount() != order->amount()) {
                        return false;
                    }
                    
                    ++order;
                }
                
                return true;
            };
    
            const auto strategy_orders = _order_manager.lock()->limitOrders();
            
            return
                is_rest_amount_correct(strategy_orders.get<Side::Buy>())  &&
                is_rest_amount_correct(strategy_orders.get<Side::Sell>());
        }
        ());
        
        assert([&] ()
        {
            const auto is_historical_depth_correct = [&] <Side S> (const Depth<S> &historical_depth)
            {
                for (auto it = historical_depth.begin(), prev = it++; it != historical_depth.end(); ++it) {
                    if (!utilits::sideLess<S>(it->price(), prev->price())) {
                        return false;
                    }
                }
    
                return true;
            };
            
            return
                is_historical_depth_correct(_historical_depth.get<Side::Buy>()) &&
                is_historical_depth_correct(_historical_depth.get<Side::Sell>());
        }
        ());
    
        assert([&] ()
        {
            const auto is_volume_before_correct = [&] <Side S> (auto strategy_orders, const Depth<S> &historical_depth)
            {
                auto level = historical_depth.begin();
                
                for (const auto &order : *strategy_orders) {
                    while (level != historical_depth.end() && utilits::sideGreater<S>(level->price(), order.price())) {
                        ++level;
                    }
                    
                    if (level->price() != order.price()) {
                        continue;
                    }
                    
                    if (order.volumeBefore() > level->volume()) {
                        return false;
                    }
                }
                
                return true;
            };
    
            const auto strategy_orders = _order_manager.lock()->limitOrders();
            
            return
                is_volume_before_correct(strategy_orders.get<Side::Buy>(), _historical_depth.get<Side::Buy>()) &&
                is_volume_before_correct(strategy_orders.get<Side::Sell>(), _historical_depth.get<Side::Sell>());
        }
        ());
    }
    
    void process(const events::OrderUpdate<OrderStatus::New> &event)
    {
        _logger.gotEvent(event);
        
        const auto &order = _order_manager.lock()->get(event.client_order_id);
    
        assert(order.type() == OrderType::Limit);
        assert(order.status() == OrderStatus::New);
        
        if (order.side() == Side::Buy) {
            placeOrder<Side::Buy>(order);
        }
        else {
            placeOrder<Side::Sell>(order);
        }
    }
    
    template <Side S>
    void process(const events::DecreaseLevel<S> &event)
    {
        _logger.gotEvent(event);
        
        auto &depth = _historical_depth.get<S>();
        auto level = depth.begin();
        while (utilits::sideGreater<S>(level->price(), event.price)) {
            ++level;
        }
        if (level->price() == event.price) {
            if (level->volume() < event.volume) {
                throw WCS_EXCEPTION(std::runtime_error, "Decreasing volume greater than level volume");
            }
            
            level->decreaseVolume(event.volume);
            
            if (!level->volume()) {
                depth.erase(level);
            }
        }
    
        assert([&] ()
        {
            const auto strategy_orders = _order_manager.lock()->limitOrders().get<S>();
            auto order = strategy_orders->begin();
            while (order != strategy_orders->end() &&utilits::sideGreater<S>(order->price(), level->price())) {
                ++order;
            }
            while (order != strategy_orders->end() && order->price() == level->price()) {
                if (order->volumeBefore() > level->volume()) {
                    return false;
                }

                ++order;
            }

            return true;
        }
        ());
    }
    
    template <Side S>
    Amount getVolume(const Price &price) const
    {
        const auto &depth = _historical_depth.get<S>();
        for (const auto &level : depth) {
            if (utilits::sideGreaterEqual<S>(price, level.price())) {
                if (price == level.price()) {
                    return level.volume();
                }
    
                break;
            }
        }
        
        return Amount { 0 };
    }
    
    const SidePair<Depth> &historicalDepth() const
    {
        return _historical_depth;
    }
    
private:
    template <Side S>
    void placeOrder(const OrderHandler &order)
    {
        if (!isIn<S>(order.price())) {
            _logger.debug(R"(Order: {}, out of order book, it will be freezed)", order);
        
            generateFreezeOrder(order.id());
        }
        else {
            generateMoveOrderTo(order.id(), getVolume<S>(order.price()));
        }
    }
    
    template <Side S>
    bool isIn(const Price &price) const
    {
        return utilits::sideLessEqual<S>(_historical_depth.get<S>().back().price(), price);
    }
    
    void moveOrders(const events::OrderBookUpdate &event) const
    {
        moveOrders(event.depth.get<Side::Buy>());
        moveOrders(event.depth.get<Side::Sell>());
    }
    
    template <Side S>
    void moveOrders(const Depth<S> &depth_update) const
    {
        const auto &historical_depth = _historical_depth.get<S>();
        const auto strategy_orders = _order_manager.lock()->limitOrders().get<S>();
    
        assert(!historical_depth.empty() || strategy_orders->empty());
        
        auto level_update = depth_update.begin();
        auto level = historical_depth.begin();
        auto order = strategy_orders->begin();
        while(order != strategy_orders->end()) {
            if (order->isFreezed()) {
                break;
            }
            
            if (utilits::sideLess<S>(order->price(), depth_update.back().price())) {
                break;
            }
            
            if (order->volumeBefore()) {
                while (utilits::sideGreater<S>(level_update->price(), order->price())) {
                    ++level_update;
                }
                while (utilits::sideGreater<S>(level->price(), order->price())) {
                    ++level;
                }
    
                if (level_update->volume() < level->volume()) {
                    
                    // TODO: make two as argument
                    const auto decrease_by = (1.0 + level_update->volume() / level->volume()) / 2.0;
                    auto new_volume_before = order->volumeBefore() * decrease_by;
                    
                    if (new_volume_before > level_update->volume()) {
                        new_volume_before = level_update->volume();
                    }
                    
                    generateMoveOrderTo(order->id(), new_volume_before);
                }
            }
            
            ++order;
        }
    }
    
    void updateHistoricalDepth(const events::OrderBookUpdate &event)
    {
        updateHistoricalDepth(event.depth.get<Side::Buy>());
        updateHistoricalDepth(event.depth.get<Side::Sell>());
    }
    
    template <Side S>
    void updateHistoricalDepth(const Depth<S> &depth)
    {
        auto &historical_depth = _historical_depth.get<S>();
        
        auto level_update = depth.begin();
        auto level = historical_depth.begin();
        while (level_update != depth.end() && level != historical_depth.end()) {
            if (utilits::sideGreater<S>(level_update->price(), level->price())) {
                level = historical_depth.insert(level, *level_update);
            }
            else if (utilits::sideLess<S>(level_update->price(), level->price())) {
                level = historical_depth.erase(level);
    
                continue;
            }
            else {
                level->updateVolume(level_update->volume());
            }
    
            ++level_update;
            ++level;
        }
    
        while (level != historical_depth.end()) {
            level = historical_depth.erase(level);
        }
    
        while (level_update != depth.end()) {
            historical_depth.push_back(*level_update);
            
            ++level_update;
        }
    }
    
    void updateFreezedOrders() const
    {
        updateFreezedOrders<Side::Buy>();
        updateFreezedOrders<Side::Sell>();
    }
    
    template <Side S>
    void updateFreezedOrders() const
    {
        const auto &historical_depth = _historical_depth.get<S>();
        const auto strategy_orders = _order_manager.lock()->limitOrders().get<S>();
        
        auto order = strategy_orders->begin();
        if (!historical_depth.empty()) {
            while (order != strategy_orders->end()) {
                if (utilits::sideLess<S>(order->price(), historical_depth.back().price())) {
                    break;
                }
                
                if (order->isFreezed()) {
                    generateUnfreezeOrder(order->id());
                    generateMoveOrderTo(order->id(), std::min(getVolume<S>(order->price()), order->volumeBefore()));
                }
                
                ++order;
            }
        }
    
        while (order != strategy_orders->end()) {
            if (order->isFreezed()) {
                break; // all next orders has been freezed before
            }
        
            _logger.debug(R"(Order: {}, out of order book, it will be freezed)", *order);
        
            generateFreezeOrder(order->id());
        
            ++order;
        }
    }
    
    void completeDepth(events::OrderBookUpdate &event) const
    {
        completeDepth(event.depth.get<Side::Buy>());
        completeDepth(event.depth.get<Side::Sell>());
    }
    
    template <Side S>
    void completeDepth(Depth<S> &depth) const
    {
        const auto &historical_depth = _historical_depth.get<S>();
        const auto strategy_orders = _order_manager.lock()->limitOrders().get<S>();
    
        auto historical_level = historical_depth.begin();
        auto level = depth.begin();
        auto order = strategy_orders->begin();
    
        while (historical_level != historical_depth.end() && level != depth.end()) {
            while (order != strategy_orders->end() && utilits::sideGreater<S>(order->price(), level->price())) {
                assert(isExecution(order->status()) && !order->isFreezed());
                
                level = depth.insert(level, Level<S> { order->price(), order->restAmount() });
        
                ++order;
        
                while (order != strategy_orders->end() && level->price() == order->price()) {
                    assert(isExecution(order->status()) && !order->isFreezed());
                    
                    level->increaseVolume(order->restAmount());
            
                    ++order;
                }
        
                if (utilits::sideGreater<S>(level->price(), historical_level->price())) {
                    ++level;
                }
            }
            
            if (utilits::sideLess<S>(level->price(), historical_level->price())) {
                level = depth.insert(level, *historical_level);
            }
            else if (utilits::sideGreater<S>(level->price(), historical_level->price())) {
                level = depth.erase(level);
    
                continue;
            }
            else {
                level->updateVolume(historical_level->volume());
            }
    
            while (order != strategy_orders->end() && order->price() == level->price()) {
                assert(isExecution(order->status()) && !order->isFreezed());
                
                level->increaseVolume(order->restAmount());
    
                ++order;
            }
    
            ++historical_level;
            ++level;
        }
        
        while (level != depth.end()) {
            level = depth.erase(level);
        }
        
        while (historical_level != historical_depth.end()) {
            depth.push_back(*historical_level);
            
            ++historical_level;
        }
    }
    
    void generateMoveOrderTo(OrderId order_id, const Amount &volume_before) const
    {
        _consumer.lock()->process(
            EventBuilder::build<events::MoveOrderTo>(TimeManager::time(), order_id, volume_before));
    }
    
    void generateFreezeOrder(OrderId order_id) const
    {
        _consumer.lock()->process(
            EventBuilder::build<events::FreezeOrder>(TimeManager::time(), order_id));
    }
    
    void generateUnfreezeOrder(OrderId order_id) const
    {
        _consumer.lock()->process(
            EventBuilder::build<events::UnfreezeOrder>(TimeManager::time(), order_id));
    }
    
private:
    std::weak_ptr<Consumer_t> _consumer;
    std::weak_ptr<const OrderManager> _order_manager;
    
    SidePair<Depth> _historical_depth;

};

} // namespace wcs

#endif //WCS_ORDERBOOK_HPP
