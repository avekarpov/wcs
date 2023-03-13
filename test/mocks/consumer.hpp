#ifndef WCS_MOCKS_CONSUMER_HPP
#define WCS_MOCKS_CONSUMER_HPP

#include <wcs/order_controller.hpp>
#include <wcs/orderbook.hpp>

namespace wcs::mocks
{

template <class Derived_t = void>
class Consumer
{
    using C = std::conditional_t<std::is_void_v<Derived_t>, Consumer, Derived_t>;
    
public:
    void setOrderController(std::shared_ptr<OrderController<C>> order_controller)
    {
        _order_controller = order_controller;
    }
    
    void setOrderbook(std::shared_ptr<Orderbook<C>> orderbook)
    {
        _orderbook = orderbook;
    }
    
    template <OrderStatus OS>
    void process(const events::OrderUpdate<OS> &event)
    {
        if constexpr (OS == OrderStatus::New) {
            processToOrderbook(event);
        }
    }
    
    void process(const events::MoveOrder &event)
    {
        processToOrderController(event);
    }
    
private:
    template <class Event>
    void processToOrderController(const Event &event)
    {
        if (_order_controller.lock()) {
            _order_controller.lock()->process(event);
        }
    }
    
    template <class Event>
    void processToOrderbook(const Event &event)
    {
        if (_orderbook.lock()) {
            _orderbook.lock()->process(event);
        }
    }
    
private:
    std::weak_ptr<OrderController<C>> _order_controller;
    std::weak_ptr<Orderbook<C>> _orderbook;

};

} // namespace wcs::mocks

#endif //WCS_MOCKS_CONSUMER_HPP
