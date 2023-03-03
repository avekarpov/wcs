#ifndef WCS_MOVE_ORDER_HPP
#define WCS_MOVE_ORDER_HPP

#include "event.hpp"
#include "../entities/id.hpp"
#include "../entities/amount.hpp"

namespace wcs::events
{

struct MoveOrder : public Event
{
    OrderId client_order_id;
    
    Amount volume_before;
};

} // namespace wcs::events

#endif //WCS_MOVE_ORDER_HPP
