#ifndef WCS_SPIES_CONSUMER_HPP
#define WCS_SPIES_CONSUMER_HPP

#include <vector>
#include <variant>

#include "../mocks/consumer.hpp"

namespace wcs::spies
{

class Consumer : public mocks::Consumer<Consumer>
{
    using Base = mocks::Consumer<Consumer>;
    
public:
    void process(const events::MoveOrder &event)
    {
        _move_orders.push_back(event);
    
        Base::process(event);
    }
    
    const auto &moveOrders() const
    {
        return _move_orders;
    }
    
    template <OrderStatus OS>
    void process(const events::OrderUpdate<OS> &event)
    {
        _order_updates.push_back(event);
    
        Base::process(event);
    }
    
    const auto &orderUpdates() const
    {
        return _order_updates;
    }
    
    void clear()
    {
        _move_orders.clear();
        _order_updates.clear();
    }
    
private:
    std::vector<events::MoveOrder> _move_orders;
    
    std::vector<
        std::variant<
            events::OrderUpdate<OrderStatus::New>,
            events::OrderUpdate<OrderStatus::Placed>,
            events::OrderUpdate<OrderStatus::Partially>,
            events::OrderUpdate<OrderStatus::Filled>,
            events::OrderUpdate<OrderStatus::Canceled>,
            events::OrderUpdate<OrderStatus::Rejected>>> _order_updates;
};

} // namespace wcs::spies

#endif //WCS_SPIES_CONSUMER_HPP
