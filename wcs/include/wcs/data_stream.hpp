#ifndef WCS_DATA_STREAM_HPP
#define WCS_DATA_STREAM_HPP

#include <memory>

#include "logger.hpp"
#include "time_manager.hpp"

namespace wcs
{

class DataStreamLogger
{
protected:
    inline static Logger _logger { "DataStream" };
};

// TODO: add test
template <class TradeStream_t, class OrderBookUpdateSteam_t, class EventManager_t>
class DataStreamBase : public DataStreamLogger
{
private:
    using BacktestEngine = typename EventManager_t::ToBacktestEngine;

    template <class Stream_t>
    class StreamController
    {
    public:
        using Event = typename Stream_t::Event;

    public:
        Stream_t &stream()
        {
            return _stream;
        }

        void processNextEvent()
        {
            _last_event = _stream.getNextEvent();
        }

        const Event &getLastEvent() const
        {
            return _last_event;
        }

        bool empty() const
        {
            return _stream.empty();
        }

    private:
        Stream_t _stream;
        Event _last_event;

    };

public:
    void setEventManager(const std::shared_ptr<EventManager_t> &event_manager)
    {
        _backtest_engine = event_manager;
    }

    TradeStream_t &tradeStream()
    {
        return _trade_stream.stream();
    }

    OrderBookUpdateSteam_t &orderBookUpdateStream()
    {
        return _order_book_update_stream.stream();
    }

    void processNextEvent()
    {
        const auto &trade = _trade_stream.getLastEvent();
        const auto &order_book_update = _order_book_update_stream.getLastEvent();

        if (trade.ts < order_book_update.ts) {
            TimeManager::process(trade);

            _logger.trace("Update time by trade event: {}", trade);

            _backtest_engine.lock()->process(trade);
            _trade_stream.processNextEvent();
        }
        else {
            TimeManager::process(order_book_update);

            _logger.trace("Update time by order book update event: {}", order_book_update);

            _backtest_engine.lock()->process(order_book_update);
            _order_book_update_stream.processNextEvent();
        }
    }

    bool empty() const
    {
        return _trade_stream.empty() || _order_book_update_stream.empty();
    }

private:
    StreamController<TradeStream_t> _trade_stream;
    StreamController<OrderBookUpdateSteam_t> _order_book_update_stream;

    std::weak_ptr<BacktestEngine> _backtest_engine;
};

} // namespace wcs

#endif //WCS_DATA_STREAM_HPP
