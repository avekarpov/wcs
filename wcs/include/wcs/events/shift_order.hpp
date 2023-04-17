#ifndef WCS_SHIFT_ORDER_HPP
#define WCS_SHIFT_ORDER_HPP

#include <spdlog/fmt/fmt.h>

#include "../entities/amount.hpp"
#include "event.hpp"

namespace wcs::events
{

struct ShiftOrder : public Event
{
    static constexpr std::string_view NAME = "ShiftOrder";
    
    OrderId client_order_id;
    
    Amount volume;
};

} // namespace wcs::events

template <>
struct fmt::formatter<wcs::events::ShiftOrder>
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
    auto format(const wcs::events::ShiftOrder &event, FormatContext& ctx) const -> decltype(ctx.out())
    {
        return fmt::format_to(
            ctx.out(),
            R"({{"event": "{}", "ts": {}, "id": {}, "client_order_id": {}, "volume": {}}})",
            wcs::events::ShiftOrder::NAME, event.ts.count(), event.id, event.client_order_id, event.volume);
    }
};

#endif //WCS_SHIFT_ORDER_HPP
