#ifndef WCS_ORDER_STATUS_HPP
#define WCS_ORDER_STATUS_HPP

#include "../entities/order.hpp"
#include "event.hpp"

namespace wcs::events
{

template <Side S, OrderStatus OS>
struct OrderUpdate
{
    OrderId client_order_id;
};

template <Side S>
struct OrderUpdate<S, OrderStatus::Partially> : public OrderUpdate<S, OrderStatus::New>
{
    Amount<S> amount;
};

template <Side S>
struct OrderUpdate<S, OrderStatus::Filled> : public  OrderUpdate<S, OrderStatus::Partially>
{

};

template <Side S>
struct OrderUpdate<S, OrderStatus::Rejected> : public OrderUpdate<S, OrderStatus::New>
{
    // TODO: add reason
};

} // namespace wcs::events

#endif //WCS_ORDER_STATUS_HPP
