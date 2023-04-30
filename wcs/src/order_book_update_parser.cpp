#include "../include/wcs/event_builder.hpp"
#include "../include/wcs/utilits/convertors.hpp"
#include "../include/wcs/order_book_update_parser.hpp"

namespace wcs
{

void OrderBookUpdateParser::setDepthSize(size_t size)
{
    _depth.get<Side::Buy>().resize(size);
    _depth.get<Side::Sell>().resize(size);
}

events::OrderBookUpdate OrderBookUpdateParser::parse(std::string_view event_string)
{
    _logger.trace(R"(Parse event string "{}")", event_string);
    
    size_t from = 0;
    size_t pos = event_string.find(',');
    const Ts ts { utilits::from_string_to<uint64_t>(event_string, from, pos) };
    
    auto &buy = _depth.get<Side::Buy>();
    for (auto &level : buy)
    {
        from = pos + 2;
        pos = event_string.find(',', from);
        Price price { utilits::from_string_to<double>(event_string, from, pos) };
        
        from = pos + 2;
        pos = event_string.find(',', from);
        Amount volume { utilits::from_string_to<double>(event_string, from, pos) };
        
        level = Level<Side::Buy> { price, volume };
    }
    
    auto &sell = _depth.get<Side::Sell>();
    for (auto &level : sell)
    {
        from = pos + 2;
        pos = event_string.find(',', from);
        Price price { utilits::from_string_to<double>(event_string, from, pos) };
        
        from = pos + 2;
        pos = event_string.find(',', from);
        Amount volume { utilits::from_string_to<double>(event_string, from, pos) };
        
        level = Level<Side::Sell> { price, volume };
    }
    
    return EventBuilder::build<events::OrderBookUpdate>(ts, _depth);
}

} // namespace wcs
