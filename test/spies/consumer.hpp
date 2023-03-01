#ifndef WCS_CONSUMER_HPP
#define WCS_CONSUMER_HPP

#include <vector>
#include <variant>

#include <wcs/events/order_update.hpp>

namespace wcs
{

class Consumer
{
public:
    template <Side S, OrderStatus OS>
    void process(const events::OrderUpdate<S, OS> &event)
    {
        _order_updates.push_back(event);
    }
    
    const auto &orderUpdates() const
    {
        return _order_updates;
    }
    
private:
    std::vector<
        std::variant<
            events::OrderUpdate<Side::Buy, OrderStatus::New>,
            events::OrderUpdate<Side::Sell, OrderStatus::New>,
            events::OrderUpdate<Side::Buy, OrderStatus::Placed>,
            events::OrderUpdate<Side::Sell, OrderStatus::Placed>,
            events::OrderUpdate<Side::Buy, OrderStatus::Partially>,
            events::OrderUpdate<Side::Sell, OrderStatus::Partially>,
            events::OrderUpdate<Side::Buy, OrderStatus::Filled>,
            events::OrderUpdate<Side::Sell, OrderStatus::Filled>,
            events::OrderUpdate<Side::Buy, OrderStatus::Canceled>,
            events::OrderUpdate<Side::Sell, OrderStatus::Canceled>,
            events::OrderUpdate<Side::Buy, OrderStatus::Rejected>,
            events::OrderUpdate<Side::Sell, OrderStatus::Rejected>>> _order_updates;
};

} // namespace wcs

#endif //WCS_CONSUMER_HPP
