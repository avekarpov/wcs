#ifndef WCS_DECREASE_LEVEL_HPP
#define WCS_DECREASE_LEVEL_HPP

#include <spdlog/fmt/fmt.h>

#include "../entities/amount.hpp"
#include "../entities/price.hpp"
#include "../entities/side.hpp"
#include "event.hpp"

namespace wcs::events
{

template <Side S>
struct DecreaseLevel : public Event
{
    static constexpr std::string_view NAME = "DecreaseLevel";
    
    Price price;
    Amount volume;
};

} // namespace wcs::events

template <wcs::Side S>
struct fmt::formatter<wcs::events::DecreaseLevel<S>>
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
    auto format(const wcs::events::DecreaseLevel<S> &event, FormatContext& ctx) const -> decltype(ctx.out())
    {
        return fmt::format_to(
            ctx.out(),
            R"({{"event": "{}", "ts": {}, "id": {}, "price": {}, "volume": {}, "side": "{}"}})",
            wcs::events::DecreaseLevel<S>::NAME,
            event.ts.count(),
            event.id,
            event.price,
            event.volume,
            wcs::toString(S)
        );
    }
};

#endif //WCS_DECREASE_LEVEL_HPP
