#ifndef WCS_FREEZE_ORDER_HPP
#define WCS_FREEZE_ORDER_HPP

#include <spdlog/fmt/fmt.h>

#include "event.hpp"
#include "../entities/id.hpp"

namespace wcs::events
{

struct FreezeOrder : public Event
{
    static constexpr std::string_view NAME = "FreezeOrder";
    
    OrderId client_order_id;
};

struct UnfreezeOrder : public Event
{
    static constexpr std::string_view NAME = "UnfreezeOrder";
    
    OrderId client_order_id;
};

} // namespace wcs::events

template <>
struct fmt::formatter<wcs::events::FreezeOrder>
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
    auto format(const wcs::events::FreezeOrder &event, FormatContext& ctx) const -> decltype(ctx.out())
    {
        return fmt::format_to(
            ctx.out(),
            R"({{"event": "{}", "ts": {}, "id": {}, "client_order_id": {}}})",
            wcs::events::FreezeOrder::NAME, event.ts.count(), event.id, event.client_order_id);
    }
};

template <>
struct fmt::formatter<wcs::events::UnfreezeOrder>
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
    auto format(const wcs::events::UnfreezeOrder &event, FormatContext& ctx) const -> decltype(ctx.out())
    {
        return fmt::format_to(
            ctx.out(),
            R"({{"event": "{}", "ts": {}, "id": {}, "client_order_id": {}}})",
            wcs::events::UnfreezeOrder::NAME, event.ts.count(), event.id, event.client_order_id);
    }
};

#endif //WCS_FREEZE_ORDER_HPP
