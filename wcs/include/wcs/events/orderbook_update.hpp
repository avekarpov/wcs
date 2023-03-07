#ifndef WCS_ORDERBOOK_UPDATE_HPP
#define WCS_ORDERBOOK_UPDATE_HPP

#include <spdlog/fmt/fmt.h>

#include "event.hpp"
#include "../entities/level.hpp"

namespace wcs::events
{

struct OrderbookUpdate : public Event
{
    static constexpr std::string_view NAME = "OrderbookUpdate";
    
    OrderbookUpdate &operator=(const OrderbookUpdate &other)
    {
        ts = other.ts;
        id = other.id;
        depth = other.depth;
        
        return *this;
    }
    
    SidePair<Depth> &depth;
    
};

} // namespace wcs::events

template <>
struct fmt::formatter<wcs::events::OrderbookUpdate>
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
    auto format(const wcs::events::OrderbookUpdate &event, FormatContext& ctx) const -> decltype(ctx.out())
    {
        return fmt::format_to(
            ctx.out(),
            R"({{"event": "{}", "ts": {}, "id": {}, "depth": {{"buy": [{}], "sell": [{}]}}}})",
            wcs::events::OrderbookUpdate::NAME, event.ts.count(), event.id,
            fmt::join(event.depth.get<wcs::Side::Buy>(), ", "),
            fmt::join(event.depth.get<wcs::Side::Sell>(), ", ")
        );
    }
};

#endif //WCS_ORDERBOOK_UPDATE_HPP
