#include <catch2/catch.hpp>

#include <wcs/entities/price.hpp>
#include <wcs/utilits/side_comparison.hpp>

using namespace wcs;

TEST_CASE("SideComparison")
{
    SECTION("Buy")
    {
        CHECK(utilits::sideLess(Price<Side::Buy> { 1.1 }, Price<Side::Buy> { 1.2 }));
        CHECK_FALSE(utilits::sideLess(Price<Side::Buy> { 1.2 }, Price<Side::Buy> { 1.1 }));
    
        CHECK(utilits::sideLessEqual(Price<Side::Buy> { 1.1 }, Price<Side::Buy> { 1.1 }));
        CHECK(utilits::sideLessEqual(Price<Side::Buy> { 1.1 }, Price<Side::Buy> { 1.2 }));
        CHECK_FALSE(utilits::sideLess(Price<Side::Buy> { 1.2 }, Price<Side::Buy> { 1.1 }));
    }
    
    SECTION("Sell")
    {
        CHECK(utilits::sideLess(Price<Side::Sell> { 1.2 }, Price<Side::Sell> { 1.1 }));
        CHECK_FALSE(utilits::sideLess(Price<Side::Sell> { 1.1 }, Price<Side::Sell> { 1.2 }));
    
        CHECK(utilits::sideLessEqual(Price<Side::Sell> { 1.1 }, Price<Side::Sell> { 1.1 }));
        CHECK(utilits::sideLessEqual(Price<Side::Sell> { 1.2 }, Price<Side::Sell> { 1.1 }));
        CHECK_FALSE(utilits::sideLess(Price<Side::Sell> { 1.1 }, Price<Side::Sell> { 1.2 }));
    }
}
