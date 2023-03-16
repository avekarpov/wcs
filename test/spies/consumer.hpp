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
    void process(const events::MoveOrderTo &event)
    {
        _move_order_events.push_back(event);
    
        Base::process(event);
    }
    
    const auto &moveOrderEvents() const
    {
        return _move_order_events;
    }
    
    template <OrderStatus OS>
    void process(const events::OrderUpdate<OS> &event)
    {
        _order_update_events.push_back(event);
    
        Base::process(event);
    }
    
    const auto &orderUpdateEvents() const
    {
        return _order_update_events;
    }
    
    void process(const events::FreezeOrder &event)
    {
        _freeze_order_events.push_back(event);
    
        Base::process(event);
    }
    
    const auto &freezeOrderEvents() const
    {
        return _freeze_order_events;
    }
    
    void process(const events::UnfreezeOrder &event)
    {
        _unfreeze_order_events.push_back(event);
        
        Base::process(event);
    }
    
    const auto &unfreezeOrderEvents() const
    {
        return _unfreeze_order_events;
    }
    
    void clear()
    {
        _move_order_events.clear();
        _order_update_events.clear();
        _freeze_order_events.clear();
        _unfreeze_order_events.clear();
    }
    
private:
    std::vector<events::MoveOrderTo> _move_order_events;
    
    std::vector<
        std::variant<
            events::OrderUpdate<OrderStatus::New>,
            events::OrderUpdate<OrderStatus::Placed>,
            events::OrderUpdate<OrderStatus::Partially>,
            events::OrderUpdate<OrderStatus::Filled>,
            events::OrderUpdate<OrderStatus::Canceled>,
            events::OrderUpdate<OrderStatus::Rejected>>> _order_update_events;
    
    std::vector<events::FreezeOrder> _freeze_order_events;
    std::vector<events::UnfreezeOrder> _unfreeze_order_events;
    
};

} // namespace wcs::spies

#endif //WCS_SPIES_CONSUMER_HPP
