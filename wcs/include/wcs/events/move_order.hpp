#ifndef WCS_MOVE_ORDER_HPP
#define WCS_MOVE_ORDER_HPP

#include <spdlog/fmt/fmt.h>

#include "event.hpp"
#include "../entities/id.hpp"
#include "../entities/amount.hpp"

namespace wcs::events
{

struct MoveOrder : public Event
{
    static constexpr std::string_view Name = "MoveOrder";
    
    OrderId client_order_id;
    
    Amount volume_before;
};

} // namespace wcs::events

template <>
struct fmt::formatter<wcs::events::MoveOrder>
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
    auto format(const wcs::events::MoveOrder &event, FormatContext& ctx) const -> decltype(ctx.out())
    {
        return fmt::format_to(
            ctx.out(),
            R"(ts: {}, id: {}, client_order_id: {}, volume_before {})",
            event.ts.count(), event.id, event.client_order_id, event.volume_before);
    }
};

#endif //WCS_MOVE_ORDER_HPP
