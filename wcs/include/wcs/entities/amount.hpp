#ifndef WCS_AMOUNT_HPP
#define WCS_AMOUNT_HPP

#include "side.hpp"

namespace wcs
{

template <Side S>
class Amount : public WithSide<S, double>
{

};

} // namespace wcs

#endif //WCS_AMOUNT_HPP
