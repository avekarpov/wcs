#ifndef WCS_SIDE_COMPARISON_HPP
#define WCS_SIDE_COMPARISON_HPP

#include "../entities/side.hpp"

namespace wcs::utilits
{

template <Side S, class Value_t>
inline bool sideLess(const Value_t &lhs, const Value_t &rhs)
{
    if constexpr (S == Side::Buy) {
        return lhs < rhs;
    }
    else {
        return lhs > rhs;
    }
}

template <Side S, class Value_t>
inline bool sideLessEqual(const Value_t &lhs, const Value_t &rhs)
{
    if constexpr (S == Side::Buy) {
        return lhs <= rhs;
    }
    else {
        return lhs >= rhs;
    }
}

template <Side S, class Value_t>
inline bool sideGreater(const Value_t &lhs, const Value_t &rhs)
{
    if constexpr (S == Side::Buy) {
        return lhs > rhs;
    }
    else {
        return lhs < rhs;
    }
}

template <Side S, class Value_t>
inline bool sideGreaterEqual(const Value_t &lhs, const Value_t &rhs)
{
    if constexpr (S == Side::Buy) {
        return lhs >= rhs;
    }
    else {
        return lhs <= rhs;
    }
}

} // namespace wcs::utilits

#endif //WCS_SIDE_COMPARISON_HPP
