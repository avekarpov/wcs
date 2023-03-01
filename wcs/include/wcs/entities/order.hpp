#ifndef WCS_ORDER_HPP
#define WCS_ORDER_HPP

#include "amount.hpp"
#include "id.hpp"
#include "price.hpp"

namespace wcs
{

enum class OrderType
{
    Market = 0,
    Limit
};

enum class OrderStatus
{
    New = 0,
    Placed,
    Partially,
    Filled,
    Canceled,
    Rejected
};

template <Side S, OrderType OT>
class Order;

template <Side S>
using MarketOrder = Order<S, OrderType::Market>;

template <Side S>
using LimitOrder = Order<S, OrderType::Limit>;

template <Side S>
class Order<S, OrderType::Market>
{
private:
    friend LimitOrder<S>;
    
public:
    Order () = default;
    
    Order (Amount amount) : _amount { amount } { }
    
    const Amount &amount() const
    {
        return _amount;
    }

private:
    Amount _amount;

};

template <Side S>
class Order<S, OrderType::Limit> : public MarketOrder<S>
{
public:
    Order (Amount amount) = delete;
    
    Order(Amount amount, Price price) : MarketOrder<S> { amount }, _price { price } { }

    const Price &price() const
    {
        return _price;
    }
    
private:
    Price _price;
    
};

template <class Order_t>
class OrderHandler : public Order_t
{
public:
    template <class ... Args>
    OrderHandler (OrderId id, Args &&...args)
    :
        Order_t { std::forward<Args>(args)... },
        _id { id },
        _status { OrderStatus::New }
    {
    
    }
    
    const OrderId &id() const
    {
        return _id;
    }
    
    const OrderStatus &status() const
    {
        return _status;
    }
    
    void updateStatus(OrderStatus status)
    {
        _status = status;
    }
    
private:
    OrderId _id;
    
    OrderStatus _status;
    
};

} // namespace wcs

#endif //WCS_ORDER_HPP
