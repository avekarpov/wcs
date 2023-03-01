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
    else if constexpr (S == Side::Sell) {
        return lhs > rhs;
    }
    else {
        static_assert(S == Side::Buy || S == Side::Sell);
    }
}

template <Side S, class Value_t>
inline bool sideLessEqual(const Value_t &lhs, const Value_t &rhs)
{
    if constexpr (S == Side::Buy) {
        return lhs <= rhs;
    }
    else if constexpr (S == Side::Sell) {
        return lhs >= rhs;
    }
    else {
        static_assert(S == Side::Buy || S == Side::Sell);
    }
}

} // namespace wcs::utilits

#endif //WCS_SIDE_COMPARISON_HPP
