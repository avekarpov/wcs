#ifndef WCS_EVENT_BUILDER_HPP
#define WCS_EVENT_BUILDER_HPP

#include <utility>

#include <wcs/entities/id.hpp>
#include <wcs/entities/time.hpp>

namespace wcs
{

inline Ts getNextTs()
{
    static Ts ts { 0 };
    
    return ts++;
}

inline EventId getNextEventId()
{
    static EventId id {  };
    
    return id++;
}

template <class Event, class ... Args>
Event createEvent(Args &&...args)
{
    return Event
    {
        getNextTs(),
        getNextEventId(),
        std::forward<Args>(args)...
    };
}

} // namespace wcs

#endif //WCS_EVENT_BUILDER_HPP
