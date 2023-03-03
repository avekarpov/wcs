#ifndef WCS_PRICE_HPP
#define WCS_PRICE_HPP

#include <spdlog/fmt/fmt.h>

#include "../utilits/exception.hpp"
#include "side.hpp"

namespace wcs
{

class Price
{
public:
    Price() = default;
    
    explicit Price(double value) : _value { value } { }
    
    Price(const Price &amount) = default;
    
    inline bool operator==(const Price &other) const
    {
        return std::fabs(_value - other._value) <= EPS;
    }
    
    inline bool operator!=(const Price &other) const
    {
        return std::fabs(_value - other._value) > EPS;
    }
    
    inline bool operator<(const Price &other) const
    {
        return _value + EPS < other._value;
    }
    
    inline bool operator>(const Price &other) const
    {
        return _value - EPS > other._value;
    }
    
    inline bool operator<=(const Price &other) const
    {
        return !operator>(other);
    }
    
    inline bool operator>=(const Price &other) const
    {
        return !operator<(other);
    }
    
    inline Price operator+(const Price &other) const
    {
        return Price { _value + other._value };
    }
    
    inline Price operator-(const Price &other) const
    {
        return Price { _value - other._value };
    }
    
    inline Price &operator+=(const Price &other)
    {
        _value += other._value;
        
        return *this;
    }
    
    inline Price &operator-=(const Price &other)
    {
        _value -= other._value;
        
        return *this;
    }
    
    inline Price operator*(const Price &other) const
    {
        return Price { _value * other._value };
    }
    
    inline Price operator/(const Price &other) const
    {
        return Price { _value / other._value };
    }
    
    inline Price &operator*=(const Price &other)
    {
        _value *= other._value;
        
        return *this;
    }
    
    inline Price &operator/=(const Price &other)
    {
        _value /= other._value;
        
        return *this;
    }
    
    explicit operator double() const
    {
        return _value;
    }
    
private:
    static constexpr auto EPS = std::numeric_limits<double>::epsilon();
    
    double _value;
    
};

} // namespace wcs

template <>
struct fmt::formatter<wcs::Price>
{
    static constexpr auto parse(format_parse_context& ctx) -> decltype(ctx.begin())
    {
        auto it = ctx.begin(), end = ctx.end();
        
        if (it != end && *it != '}') {
            throw WCS_EXCEPTION(format_error, "Invalid format");
        }
        
        return it;
    }
    
    template <class FormatContext>
    auto format(const wcs::Price &price, FormatContext& ctx) const -> decltype(ctx.out())
    {
        
        return fmt::format_to(
            ctx.out(), R"({})", static_cast<double>(price));
    }
};

#endif //WCS_PRICE_HPP
