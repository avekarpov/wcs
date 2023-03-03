#ifndef WCS_ORDER_UPDATE_HPP
#define WCS_ORDER_UPDATE_HPP

#include "../entities/order.hpp"
#include "event.hpp"

namespace wcs::events
{

template <OrderStatus OS>
struct OrderUpdate : public Event
{
    OrderId client_order_id;
};

template<>
struct OrderUpdate<OrderStatus::Partially> : public OrderUpdate<OrderStatus::New>
{
    Amount amount;
};

template<>
struct OrderUpdate<OrderStatus::Filled> : public  OrderUpdate<OrderStatus::Partially>
{

};

template<>
struct OrderUpdate<OrderStatus::Rejected> : public OrderUpdate<OrderStatus::New>
{
    // TODO: add reason
};

} // namespace wcs::events

#endif //WCS_ORDER_UPDATE_HPP
