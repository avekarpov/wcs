#ifndef WCS_FILL_ORDER_HPP
#define WCS_FILL_ORDER_HPP

#include <spdlog/fmt/fmt.h>

#include "../entities/amount.hpp"
#include "../entities/id.hpp"
#include "../entities/side.hpp"
#include "../utilits/exception.hpp"
#include "event.hpp"

namespace wcs::events
{

struct FillOrder : public Event
{
    static constexpr std::string_view Name = "FillOrder";
    
    OrderId client_order_id;
    
    Amount amount;
};

} // namespace wcs::events

template <>
struct fmt::formatter<wcs::events::FillOrder>
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
    auto format(const wcs::events::FillOrder &event, FormatContext& ctx) const -> decltype(ctx.out())
    {
        
        return fmt::format_to(
            ctx.out(),
            R"(ts: {}, id: {}, client_order_id: {}, amount: {})",
            event.ts.count(), event.id, event.client_order_id, event.amount);
    }
};


#endif //WCS_FILL_ORDER_HPP
