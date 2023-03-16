#ifndef WCS_MOVE_ORDER_TO_HPP
#define WCS_MOVE_ORDER_TO_HPP

#include <spdlog/fmt/fmt.h>

#include "event.hpp"
#include "../entities/id.hpp"
#include "../entities/amount.hpp"

namespace wcs::events
{

struct MoveOrderTo : public Event
{
    static constexpr std::string_view NAME = "MoveOrderTo";
    
    OrderId client_order_id;
    
    Amount volume_before;
};

} // namespace wcs::events

template <>
struct fmt::formatter<wcs::events::MoveOrderTo>
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
    auto format(const wcs::events::MoveOrderTo &event, FormatContext& ctx) const -> decltype(ctx.out())
    {
        return fmt::format_to(
            ctx.out(),
            R"({{"event": "{}", "ts": {}, "id": {}, "client_order_id": {}, "volume_before": {}}})",
            wcs::events::MoveOrderTo::NAME, event.ts.count(), event.id, event.client_order_id, event.volume_before);
    }
};

#endif //WCS_MOVE_ORDER_TO_HPP
