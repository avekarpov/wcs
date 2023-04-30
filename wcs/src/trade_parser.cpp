#include "../include/wcs/event_builder.hpp"
#include "../include/wcs/utilits/convertors.hpp"
#include "../include/wcs/trade_parser.hpp"

namespace wcs
{

events::Trade TradeParser::parse(std::string_view event_string)
{
    _logger.trace(R"(Parse event string "{}")", event_string);
    
    size_t from = 0;
    size_t pos = event_string.find(',');
    const Ts ts { utilits::from_string_to<uint64_t>(event_string, from, pos) };
    
    from = pos + 2;
    pos = event_string.find(',', from);
    const EventId id { utilits::from_string_to<EventId>(event_string, from, pos) };
    
    from = pos + 2;
    pos = event_string.find(',', from);
    const Price price { utilits::from_string_to<double>(event_string, from, pos) };
    
    from = pos + 2;
    pos = event_string.find(',', from);
    const Amount amount { utilits::from_string_to<double>(event_string, from, pos) };
    
    from = pos + 2;
    const Side make_side = event_string[from] == 'b' ? Side::Buy : Side::Sell;
    
    return EventBuilder::build<events::Trade>(ts, id, price, amount, make_side);
}

} // namespace wcs
