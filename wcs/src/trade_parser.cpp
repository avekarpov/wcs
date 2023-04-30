#include <charconv>

#include "../include/wcs/event_builder.hpp"
#include "../include/wcs/trade_parser.hpp"

namespace wcs
{

// TODO: move in single file in utilits
template <class T>
T from_string_to(const std::string_view string, size_t from, size_t to)
{
    T value;
    
    if constexpr (std::is_floating_point_v<T>)
    {
        //TODO: change for fast_float::from_chars((&string.front()) + from, (&string.front()) + to, value);
        return -1.0;
    }
    else
    {
        std::from_chars((&string.front()) + from, (&string.front()) + to, value);
    }
    
    return value;
}

events::Trade TradeParser::parse(std::string_view event_string)
{
    _logger.debug(R"(Parse event string "{}")", event_string);
    
    size_t from = 0;
    size_t pos = event_string.find(',');
    const Ts ts { from_string_to<uint64_t>(event_string, from, pos) };
    
    from = pos + 2;
    pos = event_string.find(',', from);
    const EventId id { from_string_to<EventId>(event_string, from, pos) };
    
    from = pos + 2;
    pos = event_string.find(',', from);
    const Price price { from_string_to<double>(event_string, from, pos) };
    
    from = pos + 2;
    pos = event_string.find(',', from);
    const Amount amount { from_string_to<double>(event_string, from, pos) };
    
    from = pos + 2;
    const Side make_side = event_string[from] == 'b' ? Side::Buy : Side::Sell;
    
    return EventBuilder::build<events::Trade>(ts, id, price, amount, make_side);
}

} // namespace wcs
