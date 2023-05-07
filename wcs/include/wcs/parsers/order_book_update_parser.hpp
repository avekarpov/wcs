#ifndef WCS_ORDER_BOOK_UPDATE_PARSER_HPP
#define WCS_ORDER_BOOK_UPDATE_PARSER_HPP

#include "../events/order_book_update.hpp"
#include "../logger.hpp"

namespace wcs
{

// TODO: add test
class OrderBookUpdateParser
{
public:
    using Event = events::OrderBookUpdate;
    
    static events::OrderBookUpdate &parse(std::string_view event_string);

private:
    static events::OrderBookUpdate getEvent();

private:
    inline static size_t DEPTH_SIZE = 20;
    inline static auto _event = getEvent();

    inline static Logger _logger { "OrderBookUpdateParser" };
    
};

} // namespace wcs

#endif //WCS_ORDER_BOOK_UPDATE_PARSER_HPP
