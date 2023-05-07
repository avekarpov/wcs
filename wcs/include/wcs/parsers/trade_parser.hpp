#ifndef WCS_TRADE_PARSER_HPP
#define WCS_TRADE_PARSER_HPP

#include "../events/trade.hpp"
#include "../logger.hpp"

namespace wcs
{

// TODO: add test
class TradeParser
{
public:
    using Event = events::Trade;

    static events::Trade &parse(std::string_view event_string);

private:
    inline static events::Trade _event;

    inline static Logger _logger { "TradeParser" };
    
};


} // namespace wcs

#endif //WCS_TRADE_PARSER_HPP
