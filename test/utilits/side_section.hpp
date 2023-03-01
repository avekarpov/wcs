#ifndef WCS_SIDE_SECTION_HPP
#define WCS_SIDE_SECTION_HPP

#define SIDE_SECTION(__TEST__, ...)                            \
do {                                                           \
    SECTION(#__TEST__)                                         \
    {                                                          \
        SECTION("Buy") { __TEST__<Side::Buy>(__VA_ARGS__); }   \
        SECTION("Sell") { __TEST__<Side::Sell>(__VA_ARGS__); } \
    }                                                          \
} while(0)

#endif //WCS_SIDE_SECTION_HPP
