#ifndef WCS_SIDE_HPP
#define WCS_SIDE_HPP

#include <utility>

namespace wcs
{

enum class Side
{
    Buy = 0,
    Sell
};

template <Side S, class Value_t>
class WithSide
{
public:
    WithSide() = default;
    
    explicit WithSide(const Value_t &value) : _value { value } { }
    
    WithSide(WithSide &&with_side) : _value { std::move(with_side._value) } { }
    
    WithSide &operator=(const Value_t &value)
    {
        _value = value;
        
        return *this;
    }
    
    WithSide &operator=(const WithSide &with_side)
    {
        if (this != &with_side) {
            _value = with_side._value;
        }
        
        return *this;
    }
    
    WithSide &operator=(const WithSide &&with_side)
    {
        _value = std::move(with_side._value);
    }
    
    operator Value_t() const
    {
        return _value;
    }
    
protected:
    Value_t _value;
};

} // namespace wcs

#endif //WCS_SIDE_HPP
