#ifndef WCS_EVENT_MANAGER_HPP
#define WCS_EVENT_MANAGER_HPP

#include <queue>
#include <variant>

#include "../events/events.hpp"
#include "../time_manager.hpp"
#include "../utilits/debug.hpp"

namespace wcs
{

// TODO: add test
template <
    template <class> class VirtualExchange_t,
    template <class> class ToVirtualExchange_t,
    template <class> class BacktestEngine_t,
    template <class> class ToBacktestEngine_t,
    class TradeStream_t,
    class OrderBookUpdateStream_t>
class EventManagerBase :
    public ToVirtualExchange_t<
        EventManagerBase<
            VirtualExchange_t,
            ToVirtualExchange_t,
            BacktestEngine_t,
            ToBacktestEngine_t,
            TradeStream_t,
            OrderBookUpdateStream_t>>,
    public ToBacktestEngine_t<
        EventManagerBase<
            VirtualExchange_t,
            ToVirtualExchange_t,
            BacktestEngine_t,
            ToBacktestEngine_t,
            TradeStream_t,
            OrderBookUpdateStream_t>>
{
private:
    using ThisClass =
        EventManagerBase<
            VirtualExchange_t,
            ToVirtualExchange_t,
            BacktestEngine_t,
            ToBacktestEngine_t,
            TradeStream_t,
            OrderBookUpdateStream_t>;

    using VirtualExchange = VirtualExchange_t<ThisClass>;
    using BacktestEngine = BacktestEngine_t<ThisClass>;

public:
    using ToVirtualExchange = ToVirtualExchange_t<ThisClass>;
    using ToBacktestEngine = ToBacktestEngine_t<ThisClass>;

    friend ToVirtualExchange;
    friend ToBacktestEngine;

public:
    EventManagerBase()
    :
        #ifdef WCS_DEBUG_BUILD
        _last_processed_event_ts { 0 },
        #endif
        _delay { 0 }
    {}

    void init()
    {
        if (empty()) {
            throw WCS_EXCEPTION(std::runtime_error, "Data streams empty on init");
        }

        _next_trade = &_trade_stream.nextEvent();
        _next_order_book_update = &_order_book_update_stream.nextEvent();
    }

    void setDelay(const Time &delay)
    {
        if (delay < Time { 0 }) {
            throw WCS_EXCEPTION(std::invalid_argument, "Delay less than zero");
        }

        _delay = delay;
    }

    void setVirtualExchange(const std::shared_ptr<VirtualExchange> &virtual_exchange)
    {
        _virtual_exchange = virtual_exchange;
    }

    void setBacktestEngine(const std::shared_ptr<BacktestEngine> &backtest_engine)
    {
        _backtest_engine = backtest_engine;
    }

    TradeStream_t &tradeStream()
    {
        return _trade_stream;
    }

    OrderBookUpdateStream_t &orderBookUpdateStream()
    {
        return _order_book_update_stream;
    }

    void processNextEvent()
    {
        const std::array<Ts, 4> events_ts =
        {
            _next_trade->ts,
            _next_order_book_update->ts,
            !_queue_to_virtual_exchange.empty()
                ? ts(_queue_to_virtual_exchange.front())
                : Ts::max(),
            !_queue_to_backtest_engine.empty()
                ? ts(_queue_to_backtest_engine.front())
                : Ts::max()
        };

        size_t min = 0;
        for (size_t i = 1; i < 4; ++i) {
            if (events_ts[min] > events_ts[i]) {
                min = i;
            }
        }

        switch (min)
        {
            case 0: processTradeToBacktestEngine(); break;
            case 1: processOrderBookUpdateToBacktestEngine(); break;
            case 2: processToVirtualExchangeFromQueue(); break;
            case 4: processToBacktestEngineFromQueue(); break;
        }
    }

    bool empty() const
    {
        return _trade_stream.empty() || _order_book_update_stream.empty();
    }

private:
    using ToVirtualExchange::process;
    using ToBacktestEngine::process;

    void processTradeToBacktestEngine()
    {
#ifdef WCS_DEBUG_BUILD
        assert(_last_processed_event_ts <= _next_trade->ts);
        _last_processed_event_ts = _next_trade->ts;
#endif

        TimeManager::process(*_next_trade);
        _backtest_engine.lock()->process(*_next_trade);

        _next_trade = &_trade_stream.nextEvent();
    }

    void processOrderBookUpdateToBacktestEngine()
    {
#ifdef WCS_DEBUG_BUILD
        assert(_last_processed_event_ts <= _next_order_book_update->ts);
        _last_processed_event_ts = _next_order_book_update->ts;
#endif

        TimeManager::process(*_next_order_book_update);
        _backtest_engine.lock()->process(*_next_order_book_update);

        _next_order_book_update = &_order_book_update_stream.nextEvent();
    }

    void processToVirtualExchangeFromQueue()
    {
        std::visit([&] (const auto &event)
        {
#ifdef WCS_DEBUG_BUILD
            assert(_last_processed_event_ts <= event.ts);
            _last_processed_event_ts = event.ts;
#endif

            TimeManager::process(event);
            _virtual_exchange.lock()->process(event);


        },
        _queue_to_virtual_exchange.front());

        _queue_to_virtual_exchange.pop();
    }

    void processToBacktestEngineFromQueue()
    {
        std::visit([&] (const auto &event)
        {
#ifdef WCS_DEBUG_BUILD
            assert(_last_processed_event_ts <= event.ts);
            _last_processed_event_ts = event.ts;
#endif

            TimeManager::process(event);
            _backtest_engine.lock()->process(event);
        },
        _queue_to_backtest_engine.front());

        _queue_to_backtest_engine.pop();
    }

    template <class Event_t>
    void processToVirtualExchange(Event_t event)
    {
        event.ts += _delay;
        _queue_to_virtual_exchange.push(std::move(event));
    }

    template <class Event_t>
    void processToBacktestEngine(Event_t event)
    {
        event.ts += _delay;
        _queue_to_backtest_engine.push(std::move(event));
    }

    template<class Event_t>
    static Ts ts(const Event_t &event)
    {
        return std::visit([] (const auto &event)
        {
            return event.ts;
        },
        event);
    }

private:
#ifdef WCS_DEBUG_BUILD
    Time _last_processed_event_ts;
#endif

    Time _delay;

    TradeStream_t _trade_stream;
    const typename TradeStream_t::Event *_next_trade;
    OrderBookUpdateStream_t _order_book_update_stream;
    const typename OrderBookUpdateStream_t::Event *_next_order_book_update;

    std::weak_ptr<VirtualExchange> _virtual_exchange;
    std::queue<typename ToVirtualExchange::EventVariant> _queue_to_virtual_exchange;
    std::weak_ptr<BacktestEngine> _backtest_engine;
    std::queue<typename ToBacktestEngine::EventVariant> _queue_to_backtest_engine;

};

} // namespace wcs

#endif //WCS_EVENT_MANAGER_HPP
