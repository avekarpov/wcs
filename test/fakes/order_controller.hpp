#ifndef WCS_FAKES_ORDER_CONTROLLER_HPP
#define WCS_FAKES_ORDER_CONTROLLER_HPP

#include <wcs/events/place_order.hpp>
#include <wcs/events/cancel_order.hpp>
#include <wcs/events/fill_order.hpp>
#include <wcs/events/move_order_to.hpp>
#include <wcs/events/freeze_order.hpp>
#include <wcs/events/shift_order.hpp>

#include "order_manager.hpp"

namespace wcs::fakes
{

template <class Consumer_t>
class OrderController
{
public:
    void setConsumer(std::shared_ptr<Consumer_t> consumer)
    {
    
    }
    
    std::shared_ptr<const OrderManager> orderManager() const
    {
        return _order_manager;
    }
    
    template <Side S, OrderType OT>
    void process(const events::PlaceOrder<S, OT> &event)
    {
    
    }
    
    void process(const events::CancelOrder &event)
    {
    
    }
    
    void process(const events::FillOrder &event)
    {
    
    }
    
    void process(const events::MoveOrderTo &event)
    {
    
    }
    
    void process(const events::FreezeOrder &event)
    {
    
    }
    
    void process(const events::UnfreezeOrder &event)
    {
    
    }
    
    void process(const events::ShiftOrder &event)
    {
    
    }
    
private:
    std::shared_ptr<OrderManager> _order_manager;
    
};

} // namespace wcs::fakes

#endif //WCS_FAKES_ORDER_CONTROLLER_HPP
