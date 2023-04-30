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
    
    inline static void update(Time time)
    {
        assert(_time < time);
        
        _time = time;
    }
    
private:
    inline static Time _time { 0 };

};

} // namespace wcs

#endif //WCS_TIME_MANAGER_HPP
