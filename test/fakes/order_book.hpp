#ifndef WCS_FAKES_ORDER_BOOK_HPP
#define WCS_FAKES_ORDER_BOOK_HPP

#include <wcs/events/decrease_level.hpp>
#include <wcs/events/order_book_update.hpp>
#include <wcs/events/order_update.hpp>

#include "order_manager.hpp"

namespace wcs::fakes
{

template <class Consumer_t>
class OrderBook
{
public:
    void setConsumer(std::shared_ptr<Consumer_t> consumer)
    {
    
    }
    
    void setOrderManager(const std::shared_ptr<const OrderManager> &order_manager)
    {
    
    }
    
    void process(const events::OrderBookUpdate &event)
    {
    
    }
    
    void process(const events::OrderUpdate<OrderStatus::New> &event)
    {
    
    }
    
    template <Side S>
    void process(const events::DecreaseLevel<S> &event)
    {
    
    }

    const events::OrderBookUpdate &update() const
    {
        return _order_book_update;
    }

private:
    events::OrderBookUpdate _order_book_update;
};

} // namespace wcs::fakes

#endif //WCS_FAKES_ORDER_BOOK_HPP
