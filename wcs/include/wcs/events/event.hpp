#ifndef WCS_EVENT_HPP
#define WCS_EVENT_HPP

#include "../entities/id.hpp"
#include "../entities/time.hpp"

namespace wcs::events
{

struct Event
{
    Ts ts;
    EventId event_id;
};

} // namespace wcs::events

#endif //WCS_EVENT_HPP
