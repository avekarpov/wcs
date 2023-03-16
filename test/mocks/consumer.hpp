#ifndef WCS_MOCKS_CONSUMER_HPP
#define WCS_MOCKS_CONSUMER_HPP

#include <wcs/order_controller.hpp>
#include <wcs/order_book.hpp>

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
    
    void setOrderBook(std::shared_ptr<OrderBook<C>> order_book)
    {
        _order_book = order_book;
    }
    
    template <OrderStatus OS>
    void process(const events::OrderUpdate<OS> &event)
    {
        if constexpr (OS == OrderStatus::New) {
            processToOrderBook(event);
        }
    }
    
    void process(const events::MoveOrderTo &event)
    {
        processToOrderController(event);
    }
    
    void process(const events::FreezeOrder &event)
    {
        processToOrderController(event);
    }
    
    void process(const events::UnfreezeOrder &event)
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
    void processToOrderBook(const Event &event)
    {
        if (_order_book.lock()) {
            _order_book.lock()->process(event);
        }
    }
    
private:
    std::weak_ptr<OrderController<C>> _order_controller;
    std::weak_ptr<OrderBook<C>> _order_book;

};

} // namespace wcs::mocks

#endif //WCS_MOCKS_CONSUMER_HPP
