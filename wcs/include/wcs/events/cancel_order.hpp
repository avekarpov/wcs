#ifndef WCS_CANCEL_ORDER_HPP
#define WCS_CANCEL_ORDER_HPP

#include <spdlog/fmt/fmt.h>

#include "../entities/id.hpp"
#include "../utilits/exception.hpp"
#include "event.hpp"

namespace wcs::events
{

struct CancelOrder : public Event
{
    static constexpr std::string_view Name = "CancelOrder";
    
    OrderId client_order_id;
};

} // namespace wcs::events

template <>
struct fmt::formatter<wcs::events::CancelOrder>
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
    auto format(const wcs::events::CancelOrder &event, FormatContext& ctx) const -> decltype(ctx.out())
    {

        return fmt::format_to(
            ctx.out(),
            R"(ts: {}, id: {}, client_order_id: {})",
            event.ts.count(), event.id, event.client_order_id);
    }
};

#endif //WCS_CANCEL_ORDER_HPP
