#ifndef WCS_TO_VIRTUAL_EXCHANGE_H
#define WCS_TO_VIRTUAL_EXCHANGE_H

#include <memory>

#include "../events/trade.hpp"
#include "../events/order_book_update.hpp"
#include "../events/order_update.hpp"

namespace wcs
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
        processToVirtualExchange(event);
    }

    void process(const events::OrderBookUpdate &event)
    {
        processToVirtualExchange(event);
    }

    template<OrderStatus OS>
    void process(const events::OrderUpdate<OS> &event)
    {
        static_assert(OS != OrderStatus::New);

        processToVirtualExchange(event);
    }

private:
    template <class Event_t>
    void processToVirtualExchange(const Event_t &event)
    {
        static_cast<EventManager_t &>(*this).processToVirtualExchange(event);
    }

};

} // namespace wcs

#endif //WCS_TO_VIRTUAL_EXCHANGE_H
