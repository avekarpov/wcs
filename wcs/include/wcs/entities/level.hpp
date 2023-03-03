#ifndef WCS_LEVEL_HPP
#define WCS_LEVEL_HPP

#include "amount.hpp"
#include "price.hpp"
#include "side.hpp"

namespace wcs
{

template <Side S>
class Level
{
public:
    Level() = default;
    
    Level(const Price &price, const Amount &volume) : _price { price }, _volume { volume } { }
    
    inline bool operator==(const Level &other) const
    {
        return _price == other._price && _volume == other._volume;
    }
    
    inline bool operator!=(const Level &other) const
    {
        return !operator==(other);
    }
    
    const Price &price() const
    {
        return _price;
    }
    
    const Amount &volume() const
    {
        return _volume;
    }
    
    void updateVolume(const Amount &volume)
    {
        _volume = volume;
    }
    
private:
    Price _price;
    Amount _volume;
    
};

template <Side S>
using Depth = std::vector<Level<S>>;

} // namespace wcs

#endif //WCS_LEVEL_HPP
