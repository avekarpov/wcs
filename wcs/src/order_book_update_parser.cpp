#include "../include/wcs/event_builder.hpp"
#include "../include/wcs/utilits/convertors.hpp"
#include "../include/wcs/parsers/order_book_update_parser.hpp"

namespace wcs
{

events::OrderBookUpdate OrderBookUpdateParser::getEvent()
{
    events::OrderBookUpdate event;

    event.depth.get<Side::Buy>().resize(DEPTH_SIZE);
    event.depth.get<Side::Sell>().resize(DEPTH_SIZE);

    return event;
}

events::OrderBookUpdate &OrderBookUpdateParser::parse(std::string_view event_string)
{
    _logger.trace(R"(Parse event string "{}")", event_string);
    
    size_t from = 0;
    size_t pos = event_string.find(',');
    _event.ts = Ts {utilits::fromStringTo<uint64_t>(event_string, from, pos) };
    
    auto &buy = _event.depth.get<Side::Buy>();
    for (auto &level : buy)
    {
        from = pos + 2;
        pos = event_string.find(',', from);
        Price price {utilits::fromStringTo<double>(event_string, from, pos) };
        
        from = pos + 2;
        pos = event_string.find(',', from);
        Amount volume {utilits::fromStringTo<double>(event_string, from, pos) };
        
        level = Level<Side::Buy> { price, volume };
    }
    
    auto &sell = _event.depth.get<Side::Sell>();
    for (auto &level : sell)
    {
        from = pos + 2;
        pos = event_string.find(',', from);
        Price price {utilits::fromStringTo<double>(event_string, from, pos) };
        
        from = pos + 2;
        pos = event_string.find(',', from);
        Amount volume {utilits::fromStringTo<double>(event_string, from, pos) };
        
        level = Level<Side::Sell> { price, volume };
    }
    
    return EventBuilder::updateId<events::OrderBookUpdate>(_event);
}

} // namespace wcs
