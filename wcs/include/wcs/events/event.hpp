#ifndef WCS_EVENT_HPP
#define WCS_EVENT_HPP

#include "../entities/id.hpp"
#include "../entities/time.hpp"

namespace wcs::events
{

struct Event
{
    Ts ts;
    EventId id;
};

} // namespace wcs::events

#endif //WCS_EVENT_HPP
