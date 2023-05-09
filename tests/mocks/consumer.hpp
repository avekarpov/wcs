#ifndef WCS_MOCKS_CONSUMER_HPP
#define WCS_MOCKS_CONSUMER_HPP

#include <wcs/matching_engine.hpp>
#include <wcs/order_book.hpp>
#include <wcs/order_controller.hpp>

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
    
    void setMatchingEngine(std::shared_ptr<MatchingEngine<C, true>> matching_engine)
    {
        _matching_engine = matching_engine;
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
    
    template <Side S>
    void process(const events::DecreaseLevel<S> &event)
    {
        processToOrderBook(event);
    }
    
    void process(const events::ShiftOrder &event)
    {
        processToOrderController(event);
    }
    
    void process(const events::FillOrder &event)
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
    
    template <class Event>
    void processToMatchingEngine(const Event &event)
    {
        if (_matching_engine) {
            _matching_engine.lock()->process(event);
        }
    }
    
private:
    std::weak_ptr<OrderController<C>> _order_controller;
    std::weak_ptr<OrderBook<C>> _order_book;
    std::weak_ptr<MatchingEngine<C, true>> _matching_engine;

};

} // namespace wcs::mocks

#endif //WCS_MOCKS_CONSUMER_HPP
