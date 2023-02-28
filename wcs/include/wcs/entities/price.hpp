#ifndef WCS_PRICE_HPP
#define WCS_PRICE_HPP

#include "side.hpp"

namespace wcs
{

template <Side S>
class Price
{
public:
    Price() = default;
    
    explicit Price(double value) : _value { value } { }
    
    Price(const Price<S> &amount) : _value { amount._value } { }
    
    inline bool operator==(const Price<S> &other) const
    {
        return std::fabs(_value - other._value) <= EPS;
    }
    
    inline bool operator!=(const Price<S> &other) const
    {
        return std::fabs(_value - other._value) > EPS;
    }
    
    inline bool operator<(const Price<S> &other) const
    {
        return _value + EPS < other._value;
    }
    
    inline bool operator>(const Price<S> &other) const
    {
        return _value - EPS > other._value;
    }
    
    inline bool operator<=(const Price<S> &other) const
    {
        return !operator>(other);
    }
    
    inline bool operator>=(const Price<S> &other) const
    {
        return !operator<(other);
    }

private:
    static constexpr auto EPS = std::numeric_limits<double>::epsilon();
    
    double _value;
    
};

} // namespace wcs

#endif //WCS_PRICE_HPP
