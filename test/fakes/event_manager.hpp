#ifndef WCS_FAKES_EVENT_MANAGER_HPP
#define WCS_FAKES_EVENT_MANAGER_HPP

#include <wcs/events/events.hpp>

namespace wcs::fakes
{

class FromBacktestEngine
{
public:
    template <class Event_t>
    void process(const Event_t &event)
    {
    
    }
    
};

class FromVirtualExchange
{
public:
    template <class Event_t>
    void process(const Event_t &event)
    {
    
    }
    
};

template <class FromBacktestEngine_t, class FromVirtualExchange_t>
class EventManager : public FromBacktestEngine_t, public FromVirtualExchange_t
{
public:
    using FromBacktestEngine = FromBacktestEngine_t;
    using FromVirtualExchange = FromVirtualExchange_t;
    
};

} // namespace wcs::fakes

#endif //WCS_FAKES_EVENT_MANAGER_HPP
