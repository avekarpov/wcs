#ifndef WCS_ORDER_BOOK_UPDATE_HPP
#define WCS_ORDER_BOOK_UPDATE_HPP

#include <spdlog/fmt/fmt.h>

#include "event.hpp"
#include "../entities/level.hpp"

namespace wcs::events
{

struct OrderBookUpdate : public Event
{
    static constexpr std::string_view NAME = "OrderBookUpdate";
    
    OrderBookUpdate &operator=(const OrderBookUpdate &other)
    {
        if (this != &other) {
            ts = other.ts;
            id = other.id;
            depth = other.depth;
        }

        return *this;
    }
    
    SidePair<Depth> &depth;
};

} // namespace wcs::events

template <>
struct fmt::formatter<wcs::events::OrderBookUpdate>
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
    auto format(const wcs::events::OrderBookUpdate &event, FormatContext& ctx) const -> decltype(ctx.out())
    {
        return fmt::format_to(
            ctx.out(),
            R"({{"event": "{}", "ts": {}, "id": {}, "depth": {{"buy": [{}], "sell": [{}]}}}})",
            wcs::events::OrderBookUpdate::NAME, event.ts.count(), event.id,
            fmt::join(event.depth.get<wcs::Side::Buy>(), ", "),
            fmt::join(event.depth.get<wcs::Side::Sell>(), ", ")
        );
    }
};

#endif //WCS_ORDER_BOOK_UPDATE_HPP
