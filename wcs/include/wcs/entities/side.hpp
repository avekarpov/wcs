#ifndef WCS_SIDE_HPP
#define WCS_SIDE_HPP

#include <utility>

namespace wcs
{

enum class Side
{
    Buy = 0,
    Sell,
};

template <template <Side> class Value_t>
struct SidePair
{
    template <Side S>
    explicit operator Side()
    {
        if constexpr (S == Side::Buy) {
            return buy;
        }
        else if constexpr (S == Side::Sell) {
            return sell;
        }
        else {
            static_assert(S == Side::Buy || S == Side::Sell);
        }
    }
    
    template <Side S>
    Value_t<S> &get()
    {
        if constexpr (S == Side::Buy) {
            return buy;
        }
        else if constexpr (S == Side::Sell) {
            return sell;
        }
        else {
            static_assert(S == Side::Buy || S == Side::Sell);
        }
    }
    
    template <Side S>
    const Value_t<S> &get() const
    {
        if constexpr (S == Side::Buy) {
            return buy;
        }
        else if constexpr (S == Side::Sell) {
            return sell;
        }
        else {
            static_assert(S == Side::Buy || S == Side::Sell);
        }
    }
    
private:
    Value_t<Side::Buy> buy;
    Value_t<Side::Sell> sell;
    
};

} // namespace wcs

#endif //WCS_SIDE_HPP
