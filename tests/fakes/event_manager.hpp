#ifndef WCS_FAKES_EVENT_MANAGER_HPP
#define WCS_FAKES_EVENT_MANAGER_HPP

#include <variant>

#include <wcs/events/events.hpp>

namespace wcs::fakes
{

template <class EventManager_t>
class ToVirtualExchange
{
public:
    using EventVariant =
        std::variant<
            events::Trade,
            events::OrderBookUpdate,
            events::OrderUpdate<OrderStatus::Placed>,
            events::OrderUpdate<OrderStatus::Partially>,
            events::OrderUpdate<OrderStatus::Filled>,
            events::OrderUpdate<OrderStatus::Canceled>,
            events::OrderUpdate<OrderStatus::Rejected>>;

public:
    void process(const events::Trade &event)
    {

    }

    void process(const events::OrderBookUpdate &event)
    {

    }

    template<OrderStatus OS>
    void process(const events::OrderUpdate<OS> &event)
    {

    }
    
};

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
    void process(const events::Trade &event)
    {

    }

    void process(events::OrderBookUpdate &event)
    {

    }

    template<Side S, OrderType OT>
    void process(const events::PlaceOrder<S, OT> &event)
    {

    }

    void process(const events::CancelOrder &event)
    {

    }
    
};

template <template <class> class ToVirtualExchange_t, template <class> class ToBacktestEngine_t>
class EventManager :
    public ToVirtualExchange_t<EventManager<ToVirtualExchange_t, ToBacktestEngine_t>>,
    public ToBacktestEngine_t<EventManager<ToVirtualExchange_t, ToBacktestEngine_t>>
{
private:
    using ThisClass = EventManager<ToVirtualExchange_t, ToBacktestEngine_t>;

public:
    using ToVirtualExchange = ToVirtualExchange_t<ThisClass>;
    using ToBacktestEngine = ToBacktestEngine_t<ThisClass>;

private:
    using ToVirtualExchange::process;
    using ToBacktestEngine::process;

};

} // namespace wcs::fakes

#endif //WCS_FAKES_EVENT_MANAGER_HPP
