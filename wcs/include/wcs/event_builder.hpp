#ifndef WCS_EVENT_BUILDER_HPP
#define WCS_EVENT_BUILDER_HPP

#include <utility>

#include "entities/id.hpp"
#include "entities/time.hpp"

namespace wcs
{

// TODO: temporary solution, required make event builder not global
class EventBuilder
{
public:
    template <class Event_t, class ... Args>
    inline static Event_t build(Time time, Args &&...args)
    {
        return Event_t
        {
            time,
            _next_event_id++,
            std::forward<Args>(args)...
        };
    }

    template <class Event_t>
    inline static void updateId(Event_t &event)
    {
        event.id = _next_event_id++;
    }

private:
    inline static EventId _next_event_id { 1 };
    
};

} // namespace wcs

#endif //WCS_EVENT_BUILDER_HPP
