#ifndef WCS_ORDER_HPP
#define WCS_ORDER_HPP

#include "price.hpp"
#include "amount.hpp"

namespace wcs
{

enum class OrderType
{
    Market = 0,
    Limit
};

template <Side S, OrderType Type>
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
    Order (Amount<S> amount) : _amount { amount } { }
    
    const Amount<S> &amount() const
    {
        return _amount;
    }

private:
    Amount<S> _amount;

};

template <Side S>
class Order<S, OrderType::Limit> : public MarketOrder<S>
{
private:
    Order(Amount<S> amount, Price<S> price) : MarketOrder<S> { amount }, _price {price} { }

    const Price<S> &price() const
    {
        return _price;
    }
    
private:
    Price<S> _price;
    
};

} // namespace wcs

#endif //WCS_ORDER_HPP
