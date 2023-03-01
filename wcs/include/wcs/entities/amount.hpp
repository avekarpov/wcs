#ifndef WCS_AMOUNT_HPP
#define WCS_AMOUNT_HPP

#include "side.hpp"

namespace wcs
{

class Amount
{
public:
    Amount() = default;
    
    explicit Amount(double value) : _value { value } { }
    
    Amount(const Amount &amount) : _value { amount._value } { }
    
    inline bool operator==(const Amount &other) const
    {
        return std::fabs(_value - other._value) <= EPS;
    }
    
    inline bool operator!=(const Amount &other) const
    {
        return std::fabs(_value - other._value) > EPS;
    }
    
    inline bool operator<(const Amount &other) const
    {
        return _value + EPS < other._value;
    }
    
    inline bool operator>(const Amount &other) const
    {
        return _value - EPS > other._value;
    }
    
    inline bool operator<=(const Amount &other) const
    {
        return !operator>(other);
    }
    
    inline bool operator>=(const Amount &other) const
    {
        return !operator<(other);
    }

    inline Amount operator+(const Amount &other) const
    {
        return Amount { _value + other._value };
    }
    
    inline Amount operator-(const Amount &other) const
    {
        return Amount { _value - other._value };
    }
    
    inline Amount &operator+=(const Amount &other)
    {
        _value += other._value;
        
        return *this;
    }
    
    inline Amount &operator-=(const Amount &other)
    {
        _value -= other._value;
    
        return *this;
    }
    
    inline Amount operator*(const Amount &other) const
    {
        return Amount { _value * other._value };
    }
    
    inline Amount operator/(const Amount &other) const
    {
        return Amount { _value / other._value };
    }
    
    inline Amount &operator*=(const Amount &other)
    {
        _value *= other._value;
        
        return *this;
    }
    
    inline Amount &operator/=(const Amount &other)
    {
        _value /= other._value;
        
        return *this;
    }
    
private:
    static constexpr auto EPS = std::numeric_limits<double>::epsilon();
    
    double _value;

};

} // namespace wcs

#endif //WCS_AMOUNT_HPP
