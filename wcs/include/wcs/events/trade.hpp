#ifndef WCS_TRADE_HPP
#define WCS_TRADE_HPP

#include <spdlog/fmt/fmt.h>

#include "../entities/amount.hpp"
#include "../entities/price.hpp"
#include "../entities/side.hpp"
#include "event.hpp"

namespace wcs::events
{

struct Trade : public Event
{
    static constexpr std::string_view NAME = "Trade";
    
    TradeId trade_id;
    
    Price price;
    Amount volume;
    
    Side maker_side;
};

} // namespace wcs::events

template <>
struct fmt::formatter<wcs::events::Trade>
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
    auto format(const wcs::events::Trade &event, FormatContext& ctx) const -> decltype(ctx.out())
    {
        return fmt::format_to(
            ctx.out(),
            R"({{"event": "{}", "ts": {}, "id": {}, "trade_id": {}, "price": {}, "volume": {}, "maker_side": "{}"}})",
            wcs::events::Trade::NAME,
            event.ts.count(),
            event.id,
            event.trade_id,
            event.price,
            event.volume,
            wcs::toString(event.maker_side)
        );
    }
};

#endif //WCS_TRADE_HPP
