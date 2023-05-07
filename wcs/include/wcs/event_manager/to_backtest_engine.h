#ifndef WCS_TO_BACKTEST_ENGINE_H
#define WCS_TO_BACKTEST_ENGINE_H

#include <memory>

#include "../events/cancel_order.hpp"
#include "../events/order_book_update.hpp"
#include "../events/place_order.hpp"
#include "../events/trade.hpp"

namespace wcs
{

template <template <class> class BacktestEngine_t, class EventManager_t>
class ToBacktestEngineBase
{
public:
    void setBacktestEngine(const std::shared_ptr<BacktestEngine_t<EventManager_t>> &backtest_engine)
    {
        _backtest_engine = backtest_engine;
    }

    void process(const events::Trade &event)
    {
        _backtest_engine.lock()->process(event);
    }

    void process(const events::OrderBookUpdate &event)
    {
        _backtest_engine.lock()->process(event);
    }

    template<Side S, OrderType OT>
    void process(const events::PlaceOrder<S, OT> &event)
    {
        _backtest_engine.lock()->process(event);
    }

    void process(const events::CancelOrder &event)
    {
        _backtest_engine.lock()->process(event);
    }

private:
    std::weak_ptr<BacktestEngine_t<EventManager_t>> _backtest_engine;
};

} // namespace wcs

#endif //WCS_TO_BACKTEST_ENGINE_H
