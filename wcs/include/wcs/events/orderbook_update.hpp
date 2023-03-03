#ifndef WCS_ORDERBOOK_UPDATE_HPP
#define WCS_ORDERBOOK_UPDATE_HPP

#include "event.hpp"
#include "../entities/level.hpp"

namespace wcs::events
{

struct OrderbookUpdate : public Event
{
    SidePair<Depth> &depth;
    
};

} // namespace wcs::events

#endif //WCS_ORDERBOOK_UPDATE_HPP
