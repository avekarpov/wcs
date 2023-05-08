#ifndef WCS_TO_BACKTEST_ENGINE_H
#define WCS_TO_BACKTEST_ENGINE_H

#include <variant>
#include <memory>

#include "../events/cancel_order.hpp"
#include "../events/order_book_update.hpp"
#include "../events/place_order.hpp"
#include "../events/trade.hpp"

namespace wcs
{

template <class EventManager_t>
class ToBacktestEngine
{
public:
    using EventVariant =
        std::variant<
            events::PlaceOrder<Side::Buy, OrderType::Limit>,
            events::PlaceOrder<Side::Sell, OrderType::Limit>,
            events::PlaceOrder<Side::Buy, OrderType::Market>,
            events::PlaceOrder<Side::Sell, OrderType::Market>,
            events::CancelOrder>;

public:
    template<Side S, OrderType OT>
    void process(const events::PlaceOrder<S, OT> &event)
    {
        processToBacktestEngine(event);
    }

    void process(const events::CancelOrder &event)
    {
        processToBacktestEngine(event);
    }

private:
    template <class Event_t>
    void processToBacktestEngine(const Event_t &event)
    {
        static_cast<EventManager_t &>(*this).processToBacktestEngine(event);
    }

};

} // namespace wcs

#endif //WCS_TO_BACKTEST_ENGINE_H
