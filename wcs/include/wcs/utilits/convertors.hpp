#ifndef WCS_CONVERTORS_HPP
#define WCS_CONVERTORS_HPP

#include <charconv>
#include <string>

namespace wcs::utilits
{

// TODO: move in single file in utilits
template <class T>
T from_string_to(const std::string_view string, size_t from, size_t to)
{
    T value;
    
    if constexpr (std::is_floating_point_v<T>)
    {
        //TODO: change for fast_float::from_chars((&string.front()) + from, (&string.front()) + to, value);
        return -1.0;
    }
    else
    {
        std::from_chars((&string.front()) + from, (&string.front()) + to, value);
    }
    
    return value;
}

}

#endif //WCS_CONVERTORS_HPP
