#ifndef WCS_ORDER_HPP
#define WCS_ORDER_HPP

#include <spdlog/fmt/fmt.h>

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

constexpr std::string_view toString(OrderType order_type)
{
    switch (order_type)
    {
        case OrderType::Market: return "Market";
        case OrderType::Limit: return "Limit";
        default: throw WCS_EXCEPTION(std::invalid_argument, "Out of enum class");
    }
}

enum class OrderStatus
{
    New = 0, // internal status
    Placed,
    Partially,
    Filled,
    Canceled,
    Rejected
};

constexpr bool isExecution(OrderStatus order_status)
{
    switch (order_status)
    {
        case OrderStatus::New:
        case OrderStatus::Filled:
        case OrderStatus::Canceled:
        case OrderStatus::Rejected:
            return false;
    
        case OrderStatus::Placed:
        case OrderStatus::Partially:
            return true;
            
        default: throw WCS_EXCEPTION(std::invalid_argument, "Out of enum class");
    }
}

constexpr bool isFinished(OrderStatus order_status)
{
    switch (order_status)
    {
        case OrderStatus::New:
        case OrderStatus::Placed:
        case OrderStatus::Partially:
            return false;
            
        case OrderStatus::Filled:
        case OrderStatus::Canceled:
        case OrderStatus::Rejected:
            return true;
        
        default: throw WCS_EXCEPTION(std::invalid_argument, "Out of enum class");
    }
}

constexpr std::string_view toString(OrderStatus order_status)
{
    switch (order_status)
    {
        case OrderStatus::New: return "New";
        case OrderStatus::Placed: return "Placed";
        case OrderStatus::Partially: return "Partially";
        case OrderStatus::Filled: return "Filled";
        case OrderStatus::Canceled: return "Canceled";
        case OrderStatus::Rejected: return "Rejected";
        default: throw WCS_EXCEPTION(std::invalid_argument, "Out of enum");
    }
}

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
        
        if ( amount <= Amount { 0 }) {
            throw WCS_EXCEPTION(std::invalid_argument, "Invalid order amount");
        }
    }
    
    // TODO: change price and amount
    Order(Side side, OrderType type, const Amount &amount, const Price &price)
    :
        _side { side },
        _type { type },
        _amount { amount },
        _price { price }
    {
        assert(type == OrderType::Limit);
    
        if ( amount <= Amount { 0 }) {
            throw WCS_EXCEPTION(std::invalid_argument, "Invalid order amount");
        }
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
    const Side _side;
    const OrderType _type;
    const Amount _amount;
    const Price _price;
    
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
        _wa_price { 0 },
        _filled_amount { 0 },
        _volume_before { std::numeric_limits<double>::max() },
        _is_freezed { false }
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
        assert(!_is_freezed);
        assert(_status != OrderStatus::New || (status == OrderStatus::Placed));
        assert(
            _status != OrderStatus::Placed ||
            (status == OrderStatus::Partially || status == OrderStatus::Filled || status == OrderStatus::Canceled)
        );
        assert(
            _status != OrderStatus::Partially ||
            (status == OrderStatus::Partially || status == OrderStatus::Filled || status == OrderStatus::Canceled)
        );
        assert(_status != OrderStatus::Filled);
        assert(_status != OrderStatus::Canceled);
        assert(_status != OrderStatus::Rejected);
        
        _status = status;
    }
    
    const Amount &filledAmount() const
    {
        return _filled_amount;
    }
    
    Amount restAmount() const
    {
        return _amount - _filled_amount;
    }

    const Price &waPrice() const
    {
        return _wa_price;
    }

    void fill(const Price &price, const Amount &amount)
    {
        assert(!_is_freezed);
        assert(isExecution(_status));
        
        assert(amount > Amount { 0 });
        assert(_filled_amount + amount <= _amount);

        // TODO: add test
        _wa_price = Price {(
            static_cast<double>(_wa_price) * static_cast<double>(_filled_amount) +
            static_cast<double>(price) * static_cast<double>(amount)) /
            static_cast<double>(_filled_amount + amount)
        };

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
        
        assert(!_is_freezed);
        assert(isExecution(_status) || _status == OrderStatus::New);
        
        assert(_volume_before >= volume);
        
        _volume_before = volume;
    }

    void shift(const Amount &volume)
    {
        assert(_type == OrderType::Limit);
        
        assert(!_is_freezed);
        assert(isExecution(_status) || _status == OrderStatus::New);
        
        assert(volume);
        assert(_volume_before >= volume);
        
        _volume_before -= volume;
    }
    
    bool isFreezed() const
    {
        return _is_freezed;
    }
    
    void freeze()
    {
        assert(!_is_freezed);
        
        _is_freezed = true;
    }
    
    void unfreeze()
    {
        assert(_is_freezed);
        
        _is_freezed = false;
    }
    
private:
    const OrderId _id;
    
    OrderStatus _status;
    Price _wa_price;
    Amount _filled_amount;
    Amount _volume_before; // only historical volume before
    
    bool _is_freezed;
    
};

} // namespace wcs

template <>
struct fmt::formatter<wcs::OrderHandler>
{
    constexpr auto parse(format_parse_context& ctx) -> decltype(ctx.begin())
    {
        auto it = ctx.begin(), end = ctx.end();
        
        if (it != end && *it != '}') {
            throw WCS_EXCEPTION(format_error, "Invalid format");
        }
        
        return it;
    }
    
    template <class FormatContext>
    auto format(const wcs::OrderHandler &order, FormatContext& ctx) const -> decltype(ctx.out())
    {
        if (order.type() == wcs::OrderType::Limit) {
            return fmt::format_to(
                ctx.out(),
                R"({{"id": {}, "type": "{}", "side": "{}", "price": {}, "amount": {}, )"
                R"("status": "{}", "wa_price": {}, "filled_amount": {}, "volume_before": {}, "is_freezed": {}}})",
                order.id(),
                wcs::toString(order.type()),
                wcs::toString(order.side()),
                order.price(),
                order.amount(),
                wcs::toString(order.status()),
                order.waPrice(),
                order.filledAmount(),
                order.volumeBefore(),
                order.isFreezed()
            );
        }
        else {
            return fmt::format_to(
                ctx.out(),
                R"({{"id": {}, "type": "{}", "side": "{}", "amount": {}, )"
                R"("status": "{}", "wa_price": {}, "filled_amount": {}, "is_freezed": {}}})",
                order.id(),
                wcs::toString(order.type()),
                wcs::toString(order.side()),
                order.amount(),
                wcs::toString(order.status()),
                order.waPrice(),
                order.filledAmount(),
                order.isFreezed()
            );
        }
    }
};

#endif //WCS_ORDER_HPP
