#ifndef WCS_BACKTEST_ENGINE_HPP
#define WCS_BACKTEST_ENGINE_HPP

#include <memory>

#include "events/events.hpp"

namespace wcs
{

template <
    template <class> class OrderController_t,
    template <class> class OrderBook_t,
    template <class> class MatchingEngine_t,
    class EventManager_t>
class BacktestEngineBase :
    public std::enable_shared_from_this<
        BacktestEngineBase<OrderController_t, OrderBook_t, MatchingEngine_t, EventManager_t>>
{
private:
    using ThisClass = BacktestEngineBase<OrderController_t, OrderBook_t, MatchingEngine_t, EventManager_t>;
    using std::enable_shared_from_this<ThisClass>::shared_from_this;
    
    using VirtualExchange = typename EventManager_t::ToVirtualExchange;
    
public:
    void setEventManager(const std::shared_ptr<EventManager_t> &event_manager)
    {
        _virtual_exchange = event_manager;
    }
    
    void init()
    {
        auto self = shared_from_this();
        
        _order_controller.setConsumer(self);
        auto order_manager = _order_controller.orderManager();
        
        _order_book.setConsumer(self);
        _order_book.setOrderManager(order_manager);
        
        _matching_engine.setConsumer(self);
        _matching_engine.setOrderManager(order_manager);
    }
    
    template <Side S, OrderType OT>
    void process(const events::PlaceOrder<S, OT> &event)
    {
        _order_controller.process(event);
    }
    
    void process(const events::CancelOrder &event)
    {
        _order_controller.process(event);
    }
    
    template <Side S>
    void process(const events::DecreaseLevel<S> &event)
    {
        _order_book.process(event);
    }
    
    void process(const events::FillOrder &event)
    {
        _order_controller.process(event);
    }
    
    void process(const events::FreezeOrder &event)
    {
        _order_controller.process(event);
    }
    
    void process(const events::UnfreezeOrder &event)
    {
        _order_controller.process(event);
    }
    
    void process(const events::MoveOrderTo &event)
    {
        _order_controller.process(event);
    }
    
    void processAndComplete(events::OrderBookUpdate &event)
    {
        _order_book.processAndComplete(event);
    
        _virtual_exchange.lock()->process(std::as_const(event));
    }
    
    template <OrderStatus OS>
    void process(const events::OrderUpdate<OS> &event)
    {
        if constexpr (OS == OrderStatus::New) {
            _order_book.process(event);
        }
        else {
            _virtual_exchange.lock()->process(event);
        }
    }
    
    void process(const events::ShiftOrder &event)
    {
        _order_controller.process(event);
    }
    
    void process(const events::Trade &event)
    {
        _matching_engine.process(event);
    
        _virtual_exchange.lock()->process(event);
    }
    
private:
    OrderController_t<ThisClass> _order_controller;
    OrderBook_t<ThisClass> _order_book;
    MatchingEngine_t<ThisClass> _matching_engine;
    
    std::weak_ptr<VirtualExchange> _virtual_exchange;
};

} // namespace wcs

#endif //WCS_BACKTEST_ENGINE_HPP
