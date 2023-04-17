#ifndef WCS_LEVEL_HPP
#define WCS_LEVEL_HPP

#include <list>

#include <spdlog/fmt/fmt.h>

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
        assert(volume >= Amount { 0 });
        
        _volume = volume;
    }
    
    void increaseVolume(const Amount &volume)
    {
        assert(volume >= Amount { 0 });
        
        _volume += volume;
    }
    
    void decreaseVolume(const Amount &volume)
    {
        assert(_volume >= volume);
        
        _volume -= volume;
    }
    
private:
    Price _price;
    Amount _volume;
    
};

// TODO: make non copyable
template <Side S>
using Depth = std::list<Level<S>>;

} // namespace wcs

template <wcs::Side S>
struct fmt::formatter<wcs::Level<S>>
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
    auto format(const wcs::Level<S> &level, FormatContext& ctx) const -> decltype(ctx.out())
    {
        return fmt::format_to(
            ctx.out(),
            R"({{"side": "{}", "price": {}, "volume": {}}})",
            wcs::toString(S), level.price(), level.volume()
        );
    }
};

#endif //WCS_LEVEL_HPP
