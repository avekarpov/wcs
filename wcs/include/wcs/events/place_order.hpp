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
    static constexpr std::string_view NAME = "PlaceOrder";
    
    OrderId client_order_id;
    
    Amount amount;
};

template <Side S>
struct PlaceOrder<S, OrderType::Limit> : public Event
{
    static constexpr std::string_view NAME = "PlaceOrder";
    
    OrderId client_order_id;
    
    Price price;
    Amount amount;
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
                R"({{"event": "{}", "ts": {}, "id": {}, "side": "{}", "type": "{}", "client_order_id": {}, )"
                R"("amount": {}}})",
                wcs::events::PlaceOrder<S, OT>::NAME, event.ts.count(), event.id, wcs::toString(S), wcs::toString(OT),
                event.client_order_id, event.amount);
        }
        else {
            return fmt::format_to(
                ctx.out(),
                R"({{"event": "{}", "ts": {}, "id": {}, "side": "{}", "type": "{}", "client_order_id": {}, )"
                R"("price": {}, "amount": {}}})",
                wcs::events::PlaceOrder<S, OT>::NAME, event.ts.count(), event.id, wcs::toString(S), wcs::toString(OT),
                event.client_order_id, event.price, event.amount);
        }
    }
};

#endif //WCS_PLACE_ORDER_HPP
