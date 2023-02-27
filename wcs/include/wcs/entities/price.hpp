#ifndef WCS_PRICE_HPP
#define WCS_PRICE_HPP

#include "side.hpp"

namespace wcs
{

template <Side S>
class Price : public WithSide<S, double>
{

};

} // namespace wcs

#endif //WCS_PRICE_HPP
