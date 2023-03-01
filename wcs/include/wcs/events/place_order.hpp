#ifndef WCS_PLACE_ORDER_HPP
#define WCS_PLACE_ORDER_HPP

#include "../entities/order.hpp"
#include "event.hpp"

namespace wcs::events
{

template <Side S, OrderType OT>
struct PlaceOrder;

template <Side S>
struct PlaceOrder<S, OrderType::Market> : public Event
{
    OrderId client_order_id;
    
    Amount amount;
};

template <Side S>
struct PlaceOrder<S, OrderType::Limit> : public PlaceOrder<S, OrderType::Market>
{
    Price price;
};

} // namespace wcs::events

#endif //WCS_PLACE_ORDER_HPP
