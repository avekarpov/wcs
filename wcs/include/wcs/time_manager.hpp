#ifndef WCS_TIME_MANAGER_HPP
#define WCS_TIME_MANAGER_HPP

#include <cassert>

#include "entities/time.hpp"

namespace wcs
{

// TODO: temporary solution, required make time manager not global
class TimeManager
{
public:
    inline static const Time &time()
    {
        return _time;
    }

    template <class Event_t>
    inline static void process(const Event_t &event)
    {
        assert(_time <= event.ts);

        _time = event.ts;
    }
    
private:
    inline static Time _time { 0 };

};

} // namespace wcs

#endif //WCS_TIME_MANAGER_HPP
