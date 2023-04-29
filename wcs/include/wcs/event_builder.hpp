#ifndef WCS_EVENT_BUILDER_HPP
#define WCS_EVENT_BUILDER_HPP

#include <cassert>
#include <utility>

#include "entities/time.hpp"
#include "entities/id.hpp"

namespace wcs
{

// TODO: temporary solution, required to split to event builder (not global) and time manager
class EventBuilder
{
public:
    inline static const Time &time()
    {
        return _time;
    }
    
    inline static void updateTime(Time time)
    {
        assert(_time <= time);
        
        _time = time;
    }
    
    template <class Event_t, class ... Args>
    inline static Event_t build(Args &&...args)
    {
        return Event_t
        {
            _time,
            _next_event_id++,
            std::forward<Args>(args)...
        };
    }
    
private:
    inline static Time _time { 0 };
    inline static EventId _next_event_id { 1 };
    
};

} // namespace wcs

#endif //WCS_EVENT_BUILDER_HPP
