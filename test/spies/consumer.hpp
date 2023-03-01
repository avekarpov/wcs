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
    template <OrderStatus OS>
    void process(const events::OrderUpdate<OS> &event)
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
            events::OrderUpdate<OrderStatus::New>,
            events::OrderUpdate<OrderStatus::Placed>,
            events::OrderUpdate<OrderStatus::Partially>,
            events::OrderUpdate<OrderStatus::Filled>,
            events::OrderUpdate<OrderStatus::Canceled>,
            events::OrderUpdate<OrderStatus::Rejected>>> _order_updates;
};

} // namespace wcs

#endif //WCS_CONSUMER_HPP
