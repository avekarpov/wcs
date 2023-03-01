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
    SidePair() = default;
    
    SidePair(const Value_t<Side::Buy> &buy, const Value_t<Side::Sell> &sell) : _buy { buy }, _sell { sell } { }
    
    template <Side S>
    Value_t<S> &get()
    {
        if constexpr (S == Side::Buy) {
            return _buy;
        }
        else if constexpr (S == Side::Sell) {
            return _sell;
        }
        else {
            static_assert(S == Side::Buy || S == Side::Sell);
        }
    }
    
    template <Side S>
    const Value_t<S> &get() const
    {
        if constexpr (S == Side::Buy) {
            return _buy;
        }
        else if constexpr (S == Side::Sell) {
            return _sell;
        }
        else {
            static_assert(S == Side::Buy || S == Side::Sell);
        }
    }
    
private:
    Value_t<Side::Buy> _buy;
    Value_t<Side::Sell> _sell;
    
};

} // namespace wcs

#endif //WCS_SIDE_HPP
