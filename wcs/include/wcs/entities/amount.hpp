#ifndef WCS_AMOUNT_HPP
#define WCS_AMOUNT_HPP

#include <spdlog/fmt/fmt.h>

#include "../utilits/exception.hpp"
#include "side.hpp"

namespace wcs
{

class Amount
{
public:
    Amount() = default;
    
    explicit Amount(double value) : _value { value } { }
    
    Amount(const Amount &amount) = default;
    
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
    
    inline double operator*(const Amount &other) const
    {
        return _value * other._value;
    }
    
    inline double operator/(const Amount &other) const
    {
        return _value / other._value;
    }
    
    inline Amount operator*(double by) const
    {
        return Amount { _value * by };
    }
    
    inline Amount operator/(double by) const
    {
        return Amount { _value / by };
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
    
    explicit operator double() const
    {
        return _value;
    }
    
    explicit operator bool() const
    {
        return static_cast<bool>(_value);
    }
    
private:
    static constexpr auto EPS = std::numeric_limits<double>::epsilon();
    
    double _value;

};

} // namespace wcs

template <>
struct fmt::formatter<wcs::Amount>
{
    constexpr auto parse(format_parse_context& ctx) -> decltype(ctx.begin())
    {
        auto it = ctx.begin(), end = ctx.end();
        
        if (it != end && *it != '}') {
            throw WCS_EXCEPTION(format_error, "Invalid format");
        }
        
        return it;
    }
    
    template <class FormatContext>
    auto format(const wcs::Amount &amount, FormatContext& ctx) const -> decltype(ctx.out())
    {
        return fmt::format_to(
            ctx.out(), R"({})", static_cast<double>(amount));
    }
};

#endif //WCS_AMOUNT_HPP
