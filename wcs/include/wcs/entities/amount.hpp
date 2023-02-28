#ifndef WCS_AMOUNT_HPP
#define WCS_AMOUNT_HPP

#include "side.hpp"

namespace wcs
{

template <Side S>
class Amount
{
public:
    Amount() = default;
    
    explicit Amount(double value) : _value { value } { }
    
    Amount(const Amount<S> &amount) : _value { amount._value } { }
    
    inline bool operator==(const Amount<S> &other) const
    {
        return std::fabs(_value - other._value) <= EPS;
    }
    
    inline bool operator!=(const Amount<S> &other) const
    {
        return std::fabs(_value - other._value) > EPS;
    }
    
    inline bool operator<(const Amount<S> &other) const
    {
        return _value + EPS < other._value;
    }
    
    inline bool operator>(const Amount<S> &other) const
    {
        return _value - EPS > other._value;
    }
    
    inline bool operator<=(const Amount<S> &other) const
    {
        return !operator>(other);
    }
    
    inline bool operator>=(const Amount<S> &other) const
    {
        return !operator<(other);
    }

private:
    static constexpr auto EPS = std::numeric_limits<double>::epsilon();
    
    double _value;

};

} // namespace wcs

#endif //WCS_AMOUNT_HPP
