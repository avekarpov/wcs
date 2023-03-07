#ifndef WCS_PLACE_ORDER_HPP
#define WCS_PLACE_ORDER_HPP

#include <spdlog/fmt/fmt.h>

#include "../entities/order.hpp"
#include "event.hpp"

namespace wcs::events
{

template <Side S, OrderType OT>
struct PlaceOrder;

template <Side S>
struct PlaceOrder<S, OrderType::Market> : public Event
{
    static constexpr std::string_view Name = "PlaceOrder";
    
    OrderId client_order_id;
    
    Amount amount;
};

template <Side S>
struct PlaceOrder<S, OrderType::Limit> : public PlaceOrder<S, OrderType::Market>
{
    Price price;
};

} // namespace wcs::events

template <wcs::Side S, wcs::OrderType OT>
struct fmt::formatter<wcs::events::PlaceOrder<S, OT>>
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
    auto format(const wcs::events::PlaceOrder<S, OT> &event, FormatContext& ctx) const -> decltype(ctx.out())
    {
        if constexpr (OT == wcs::OrderType::Market) {
            return fmt::format_to(
                ctx.out(),
                R"(ts: {}, id: {}, side: {}, type: market, client_order_id: {}, amount: {})",
                event.ts.count(), event.id, toString(S), event.client_order_id, event.amount);
        }
        else {
            return fmt::format_to(
                ctx.out(),
                R"(ts: {}, id: {}, side: {}, type: limit, client_order_id: {}, amount: {}, price: {})",
                event.ts.count(), event.id, wcs::toString(S), event.client_order_id, event.amount, event.price);
        }
    }
};

#endif //WCS_PLACE_ORDER_HPP
