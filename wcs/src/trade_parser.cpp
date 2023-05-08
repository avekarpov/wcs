#include "../include/wcs/event_builder.hpp"
#include "../include/wcs/utilits/convertors.hpp"
#include "../include/wcs/parsers/trade_parser.hpp"

namespace wcs
{

const events::Trade &TradeParser::parse(std::string_view event_string)
{
    _logger.trace(R"(Parse event string "{}")", event_string);
    
    size_t from = 0;
    size_t pos = event_string.find(',');
    _event.ts = Ts {utilits::fromStringTo<uint64_t>(event_string, from, pos) };
    
    from = pos + 2;
    pos = event_string.find(',', from);
    _event.trade_id = TradeId {utilits::fromStringTo<EventId>(event_string, from, pos) };
    
    from = pos + 2;
    pos = event_string.find(',', from);
    _event.price = Price {utilits::fromStringTo<double>(event_string, from, pos) };
    
    from = pos + 2;
    pos = event_string.find(',', from);
    _event.volume = Amount {utilits::fromStringTo<double>(event_string, from, pos) };
    
    from = pos + 2;
    _event.maker_side = event_string[from] == 'b' ? Side::Buy : Side::Sell;
    
    EventBuilder::updateId(_event);

    return _event;
}

} // namespace wcs
