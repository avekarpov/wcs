#ifndef WCS_EXCEPTION_HPP
#define WCS_EXCEPTION_HPP

#ifndef WCS_EXCEPTION_WITHOUT_PLACE

#include <spdlog/fmt/fmt.h>
#include <spdlog/fmt/compile.h>

#define STR_HELPER(__VALUE__) #__VALUE__
#define STR(__VALUE__) STR_HELPER(__VALUE__)

#define WCS_EXCEPTION(__TYPE__, __MESSAGE__) \
__TYPE__ { fmt::format(FMT_COMPILE(R"({}:{}: {})"), __FILE__, STR(__LINE__), __MESSAGE__) }

#else

#define WCS_EXCEPTION(__TYPE__, __MESSAGE__) \
__TYPE__ { __MESSAGE__ }

#endif

#endif //WCS_EXCEPTION_HPP
