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

class Order
{
public:
    Order(Side side, OrderType type, const Amount &amount)
    :
        _side { side },
        _type { type },
        _amount { amount },
        _price { }
    {
        assert(type == OrderType::Market);
    }
    
    Order(Side side, OrderType type, const Amount &amount, const Price &price)
    :
        _side { side },
        _type { type },
        _amount { amount },
        _price { price }
    {
        assert(type == OrderType::Limit);
    }
    
    Side side() const
    {
        return _side;
    }
    
    OrderType type() const
    {
        return _type;
    }
    
    const Amount &amount() const
    {
        return _amount;
    }
    
    const Price &price() const
    {
        return _price;
    }
    
protected:
    Side _side;
    OrderType _type;
    Amount _amount;
    Price _price;
    
};

class OrderHandler final : public Order
{
public:
    template <class ...Args>
    explicit OrderHandler (OrderId id, Args &&...args)
    :
        Order { std::forward<Args>(args)... },
        _id { id },
        _status { OrderStatus::New },
        _filled_amount { Amount{ 0 } }
    {
    
    }
    
    const Order &order() const
    {
        return static_cast<const Order &>(*this);
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
    
    const Amount &filledAmount() const
    {
        return _filled_amount;
    }
    
    void fill(const Amount &amount)
    {
        _filled_amount += amount;
    }
    
    const Amount &volumeBefore() const
    {
        assert(_type == OrderType::Limit);
        
        return _volume_before;
    }
    
    void updateVolumeBefore(const Amount &volume)
    {
        assert(_type == OrderType::Limit);
        
        _volume_before = volume;
    }

private:
    OrderId _id;
    
    OrderStatus _status;
    Amount _filled_amount;
    Amount _volume_before;
    
};

} // namespace wcs

#endif //WCS_ORDER_HPP
