#ifndef WCS_FAKES_MATCHING_ENGINE_HPP
#define WCS_FAKES_MATCHING_ENGINE_HPP

#include <wcs/events/trade.hpp>

#include "order_manager.hpp"

namespace wcs::fakes
{

template <class Consumer_t>
class MatchingEngine
{
public:
    void setConsumer(std::shared_ptr<Consumer_t> consumer)
    {
    
    }
    
    void setOrderManager(const std::shared_ptr<const OrderManager> &order_manager)
    {
    
    }
    
    void process(const events::Trade &event)
    {
    
    }
    
};

} // namespace wcs::fakes

#endif //WCS_FAKES_MATCHING_ENGINE_HPP
