#ifndef WCS_FILL_ORDER_HPP
#define WCS_FILL_ORDER_HPP

#include "../entities/amount.hpp"
#include "../entities/id.hpp"
#include "../entities/side.hpp"
#include "event.hpp"

namespace wcs::events
{

struct FillOrder : public Event
{
    OrderId client_order_id;
    
    Amount amount;
};

} // namespace wcs::events

#endif //WCS_FILL_ORDER_HPP
