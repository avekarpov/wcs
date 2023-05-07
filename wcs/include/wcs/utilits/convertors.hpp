#ifndef WCS_CONVERTORS_HPP
#define WCS_CONVERTORS_HPP

#include <charconv>
#include <string>

#include <fast_float/fast_float.h>

namespace wcs::utilits
{

// TODO: add test
template <class T>
T fromStringTo(const std::string_view string, size_t from, size_t to)
{
    T value;
    
    if constexpr (std::is_floating_point_v<T>) {
        fast_float::from_chars((&string.front()) + from, (&string.front()) + to, value);
    }
    else {
        std::from_chars((&string.front()) + from, (&string.front()) + to, value);
    }
    
    return value;
}

} // namespace wcs::utilits

#endif //WCS_CONVERTORS_HPP
