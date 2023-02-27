#ifndef WCS_PLACEORDER_HPP
#define WCS_PLACEORDER_HPP

#include "../entities/order.hpp"
#include "event.hpp"

namespace wcs::events
{

template <Side S, OrderType OT>
struct PlaceOrder : public Event, public Order<S, OT>
{
    OrderId client_order_id;
};

} // namespace wcs::events

#endif //WCS_PLACEORDER_HPP
