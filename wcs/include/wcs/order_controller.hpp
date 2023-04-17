#ifndef WCS_ORDER_CONTROLLER_HPP
#define WCS_ORDER_CONTROLLER_HPP

#include "events/cancel_order.hpp"
#include "events/fill_order.hpp"
#include "events/freeze_order.hpp"
#include "events/move_order_to.hpp"
#include "events/order_update.hpp"
#include "events/place_order.hpp"
#include "events/shift_order.hpp"
#include "order_manager.hpp"
#include "utilits/exception.hpp"
#include "utilits/side_comparison.hpp"
#include "logger.hpp"

namespace wcs
{

class OrderControllerLogger
{
protected:
    inline static Logger _logger { "OrderController" };
    
};

template <class Consumer>
class OrderController : public OrderControllerLogger
{
public:
    OrderController() : _order_manager { std::make_shared<OrderManager>() } { }
    
    void setConsumer(std::shared_ptr<Consumer> consumer)
    {
        _consumer = consumer;
    }
    
    std::shared_ptr<const OrderManager> orderManager() const
    {
        return _order_manager;
    }
    
    template <Side S, OrderType OT>
    void process(const events::PlaceOrder<S, OT> &event)
    {
        _logger.gotEvent(event);
        
        // TODO: check order params
        
        if constexpr (OT == OrderType::Market) {
            auto &order = _order_manager->add<S, OT>(event.client_order_id, event.amount);
            
            // Market order is placed instantly
            order.updateStatus(OrderStatus::Placed);
            
            _logger.debug(R"(Order: {}, placed)", order);
            
            generateOrderUpdate<OrderStatus::Placed>(event.client_order_id);
        }
        else {
            _order_manager->add<S, OT>(event.client_order_id, event.amount, event.price);
            
            // Limit order is placed by order book
            generateOrderUpdate<OrderStatus::New>(event.client_order_id);
        }
    }
    
    void process(const events::CancelOrder &event)
    {
        _logger.gotEvent(event);
    
        // TODO: check order params
        
        _order_manager->remove(event.client_order_id);
    
        generateOrderUpdate<OrderStatus::Canceled>(event.client_order_id);
    }
    
    void process(const events::FillOrder &event)
    {
        _logger.gotEvent(event);
        
        auto &order = _order_manager->get(event.client_order_id);
    
        order.fill(event.amount);
        
        if (!order.restAmount()) {
            order.updateStatus(OrderStatus::Filled);
            _logger.debug(R"(Order: {}, filled)", order);
            
            _order_manager->remove(event.client_order_id);
            
            generateOrderUpdate<OrderStatus::Filled>(event.client_order_id);
        }
        else {
            order.updateStatus(OrderStatus::Partially);
            _logger.debug(R"(Order: {}, partially filled)", order);
            
            generateOrderUpdate<OrderStatus::Partially>(event.client_order_id, order.filledAmount());
        }
    }
    
    void process(const events::MoveOrderTo &event)
    {
        _logger.gotEvent(event);
    
        auto &order = _order_manager->get(event.client_order_id);
    
        order.updateVolumeBefore(event.volume_before);
        
        if (order.status() == OrderStatus::New) {
            order.updateStatus(OrderStatus::Placed);
            
            _logger.info(R"(Order: {} placed)", order);
            
            generateOrderUpdate<OrderStatus::Placed>(event.client_order_id);
        }
    }
    
    void process(const events::FreezeOrder &event)
    {
        _logger.gotEvent(event);
    
        _order_manager->get(event.client_order_id).freeze();
    }
    
    void process(const events::UnfreezeOrder &event)
    {
        _logger.gotEvent(event);
        
        _order_manager->get(event.client_order_id).unfreeze();
    }
    
    void process(const events::ShiftOrder &event)
    {
        _logger.gotEvent(event);
    
        auto &order = _order_manager->get(event.client_order_id);
        
        order.shift(event.volume);
    }
    
private:
    template <OrderStatus OS, class ...Args>
    void generateOrderUpdate(Args &&...args)
    {
        _consumer.lock()->process(events::OrderUpdate<OS>
        {
            Ts { 0 }, EventId { 0 }, // TODO: change for event builder
            std::forward<Args>(args)...
        });
    }
    
private:
    std::weak_ptr<Consumer> _consumer;
    std::shared_ptr<OrderManager> _order_manager;
    
};

} // namespace wcs

#endif //WCS_ORDER_CONTROLLER_HPP
