#ifndef WCS_FAKES_EVENT_MANAGER_HPP
#define WCS_FAKES_EVENT_MANAGER_HPP

#include <wcs/events/events.hpp>

namespace wcs::fakes
{

class ToVirtualExchange
{
public:
    template <class Event_t>
    void process(const Event_t &event)
    {
    
    }
    
};

class ToBacktestEngine
{
public:
    template <class Event_t>
    void process(const Event_t &event)
    {
    
    }
    
};

template <class ToVirtualExchange_t, class ToBacktestEngine_t>
class EventManager : public ToVirtualExchange_t, public ToBacktestEngine_t
{
public:
    using ToVirtualExchange = ToVirtualExchange_t;
    using ToBacktestEngine = ToBacktestEngine_t;
    
};

} // namespace wcs::fakes

#endif //WCS_FAKES_EVENT_MANAGER_HPP
