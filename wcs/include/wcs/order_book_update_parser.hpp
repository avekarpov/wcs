#ifndef WCS_ORDER_BOOK_UPDATE_PARSER_HPP
#define WCS_ORDER_BOOK_UPDATE_PARSER_HPP

#include "events/order_book_update.hpp"
#include "logger.hpp"

namespace wcs
{

class OrderBookUpdateParser
{
public:
    using Event = events::OrderBookUpdate;
    
    static void setDepthSize(size_t size);
    
    static events::OrderBookUpdate parse(std::string_view event_string);

private:
    inline static SidePair<Depth> _depth;
    
    inline static Logger _logger { "OrderBookUpdateParser" };
    
};

} // namespace wcs

#endif //WCS_ORDER_BOOK_UPDATE_PARSER_HPP
