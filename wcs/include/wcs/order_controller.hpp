#ifndef WCS_ORDER_CONTROLLER_HPP
#define WCS_ORDER_CONTROLLER_HPP

#include "events/cancel_order.hpp"
#include "events/fill_order.hpp"
#include "events/order_update.hpp"
#include "events/place_order.hpp"
#include "order_manager.hpp"
#include "utilits/exception.hpp"
#include "utilits/side_comparison.hpp"

namespace wcs
{

template <class Consumer>
class OrderController
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
        // TODO: check order params
        
        if constexpr (OT == OrderType::Market) {
            _order_manager->add<S, OT>(event.client_order_id, event.amount);
        }
        else {
            _order_manager->add<S, OT>(event.client_order_id, event.amount, event.price);
        }
    
        generateOrderUpdate<OrderStatus::New>(event.client_order_id);
    }
    
    void process(const events::CancelOrder &event)
    {
        _order_manager->remove(event.client_order_id);
    
        generateOrderUpdate<OrderStatus::Canceled>(event.client_order_id);
    }
    
    void process(const events::FillOrder &event)
    {
        auto &orderHandler = _order_manager->get(event.client_order_id);
    
        assert(orderHandler.filledAmount() + event.amount <= orderHandler.amount());
    
        orderHandler.fill(event.amount);
        
        if (orderHandler.filledAmount() == orderHandler.amount()) {
            _order_manager->remove(event.client_order_id);
    
            generateOrderUpdate<OrderStatus::Filled>(event.client_order_id);
        }
        else {
            generateOrderUpdate<OrderStatus::Partially>(event.client_order_id, orderHandler.filledAmount());
        }
    }
    
private:
    template <OrderStatus OS, class ...Args>
    void generateOrderUpdate(Args &&...args)
    {
        _consumer.lock()->process(events::OrderUpdate<OS>
        {
            std::forward<Args>(args)...
        });
    }
    
private:
    std::weak_ptr<Consumer> _consumer;
    std::shared_ptr<OrderManager> _order_manager;
    
};

} // namespace wcs

#endif //WCS_ORDER_CONTROLLER_HPP
