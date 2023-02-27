#ifndef WCS_TIME_HPP
#define WCS_TIME_HPP

#include <chrono>

namespace wcs
{

using Microseconds = std::chrono::microseconds;
using Milliseconds = std::chrono::milliseconds;
using Seconds = std::chrono::seconds;
using Minutes = std::chrono::minutes;

using Ts = Milliseconds;

} // namespace wcs

#endif //WCS_TIME_HPP
