#ifndef WCS_CANCEL_ORDER_HPP
#define WCS_CANCEL_ORDER_HPP

#include "../entities/id.hpp"
#include "event.hpp"

namespace wcs::events
{

struct CancelOrder : public Event
{
    OrderId client_order_id;
};

}

#endif //WCS_CANCEL_ORDER_HPP
