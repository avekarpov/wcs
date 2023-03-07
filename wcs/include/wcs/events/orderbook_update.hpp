#ifndef WCS_ORDERBOOK_UPDATE_HPP
#define WCS_ORDERBOOK_UPDATE_HPP

#include "event.hpp"
#include "../entities/level.hpp"

namespace wcs::events
{

struct OrderbookUpdate : public Event
{
    OrderbookUpdate &operator=(const OrderbookUpdate &other)
    {
        ts = other.ts;
        id = other.id;
        depth = other.depth;
        
        return *this;
    }
    
    SidePair<Depth> &depth;
    
};

} // namespace wcs::events

#endif //WCS_ORDERBOOK_UPDATE_HPP
