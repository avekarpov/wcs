#ifndef WCS_EVENT_MANAGER_HPP
#define WCS_EVENT_MANAGER_HPP

#include "event_builder.hpp"
#include "events/events.hpp"

namespace wcs
{

// TODO: remove it after in single file
template <class EventManager_t, template <class> class VirtualExchange_t>
class ToVirtualExchange
{
public:
    void setVirtualExchange(const std::shared_ptr<VirtualExchange_t<EventManager_t>> &virtual_exchange)
    {
        _virtual_exchange = virtual_exchange;
    }
    
    void process(const events::Trade &event)
    {
        _virtual_exchange.lock()->process(event);
    }
    
    void process(const events::OrderBookUpdate &event)
    {
        _virtual_exchange.lock()->process(event);
    }
    
    template <OrderStatus OS>
    void process(const events::OrderUpdate<OS> &event)
    {
        static_assert(OS != OrderStatus::New);
    
        _virtual_exchange.lock()->process(event);
    }
    
private:
    std::weak_ptr<VirtualExchange_t<EventManager_t>> _virtual_exchange;
    
};

// TODO: remove it after in single file
template <class EventManager_t, template <class> class BacktestEngine_t>
class ToBacktestEngine
{
public:
    void setBacktestEngine(const std::shared_ptr<BacktestEngine_t<EventManager_t>> &backtest_engine)
    {
        _backtest_engine = backtest_engine;
    }
    
    void process(const events::Trade &event)
    {
        updateTime(event);
        
        _backtest_engine.lock()->process(event);
    }
    
    void processAndComplete(events::OrderBookUpdate &event)
    {
        updateTime(event);
        
        _backtest_engine.lock()->processAndComplete(event);
    }
    
    template <Side S, OrderType OT>
    void process(const events::PlaceOrder<S, OT> &event)
    {
        updateTime(event);
    
        _backtest_engine.lock()->process(event);
    }
    
    void process(const events::CancelOrder &event)
    {
        updateTime(event);
    
        _backtest_engine.lock()->process(event);
    }
    
private:
    template <class Event_t>
    void updateTime(const Event_t &event)
    {
        static_cast<EventManager_t &>(*this).updateTime(event.ts);
    }
    
private:
    std::weak_ptr<BacktestEngine_t<EventManager_t>> _backtest_engine;
    
};

template <
    template <class, template <class> class> class ToVirtualExchange_t,
    template <class> class VirtualExchange_t,
    template <class, template <class> class> class ToBacktestEngine_t,
    template <class> class BacktestEngine_t>
class EventManagerBase :
    public ToVirtualExchange_t<
        EventManagerBase<ToVirtualExchange_t, VirtualExchange_t, ToBacktestEngine_t, BacktestEngine_t>,
        VirtualExchange_t>,
    public ToBacktestEngine_t<
        EventManagerBase<ToVirtualExchange_t, VirtualExchange_t, ToBacktestEngine_t, BacktestEngine_t>,
        BacktestEngine_t>
{
private:
    using ThisClass = EventManagerBase<ToVirtualExchange_t, VirtualExchange_t, ToBacktestEngine_t, BacktestEngine_t>;
    
public:
    using FromBacktestEngine = ToVirtualExchange_t<ThisClass, VirtualExchange_t>;
    using FromVirtualExchange = ToBacktestEngine_t<ThisClass, BacktestEngine_t>;
    
    friend FromBacktestEngine;
    friend FromVirtualExchange;

private:
    void updateTime(Time time)
    {
        EventBuilder::updateTime(time);
    }
    
};

} // namespace wcs

#endif //WCS_EVENT_MANAGER_HPP
