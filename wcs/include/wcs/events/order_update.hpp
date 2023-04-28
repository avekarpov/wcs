#ifndef WCS_ORDER_UPDATE_HPP
#define WCS_ORDER_UPDATE_HPP

#include <spdlog/fmt/fmt.h>

#include "../entities/order.hpp"
#include "event.hpp"

namespace wcs::events
{

template <OrderStatus OS>
struct OrderUpdate : public Event
{
    static constexpr std::string_view NAME = "OrderUpdate";
    
    OrderId client_order_id;
};

template <>
struct OrderUpdate<OrderStatus::Partially> : public OrderUpdate<OrderStatus::New>
{
    Amount amount;
};

template <>
struct OrderUpdate<OrderStatus::Filled> : public  OrderUpdate<OrderStatus::Partially>
{

};

template <>
struct OrderUpdate<OrderStatus::Rejected> : public OrderUpdate<OrderStatus::New>
{
    // TODO: add reason
};

} // namespace wcs::events

template <wcs::OrderStatus OS>
struct fmt::formatter<wcs::events::OrderUpdate<OS>>
{
    constexpr auto parse(format_parse_context& ctx) -> decltype(ctx.begin())
    {
        auto it = ctx.begin(), end = ctx.end();
        
        if (it != end && *it != '}') {
            throw WCS_EXCEPTION(format_error, "Invalid format");
        }
        
        return it;
    }
    
    template <class FormatContext>
    auto format(const wcs::events::OrderUpdate<OS> &event, FormatContext& ctx) const -> decltype(ctx.out())
    {
        if constexpr (OS == wcs::OrderStatus::New || OS == wcs::OrderStatus::Canceled) {
            return fmt::format_to(
                ctx.out(),
                R"({{"event": "{}", "ts": {}, "id": {}, "status": "{}", "client_order_id": {}}})",
                wcs::events::OrderUpdate<OS>::NAME, event.ts.count(), event.id, wcs::toString(OS),
                event.client_order_id);
        }
        else if constexpr (OS == wcs::OrderStatus::Partially || OS == wcs::OrderStatus::Filled) {
            return fmt::format_to(
                ctx.out(),
                R"({{"event": "{}", "ts": {}, "id": {}, "status": "{}", "client_order_id": {}, "amount": {}}})",
                wcs::events::OrderUpdate<OS>::NAME, event.ts.count(), event.id, wcs::toString(OS),
                event.client_order_id, event.amount);
        }
        else {
            return fmt::format_to(
                ctx.out(),
                R"({{"event": "{}", "ts": {}, "id": {}, "status": "{}", "client_order_id": {}}})",
                wcs::events::OrderUpdate<OS>::NAME, event.ts.count(), event.id, wcs::toString(OS),
                event.client_order_id);
        }
    }
};

#endif //WCS_ORDER_UPDATE_HPP
