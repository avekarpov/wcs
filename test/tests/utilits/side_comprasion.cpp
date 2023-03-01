#include <catch2/catch.hpp>

#include <wcs/entities/price.hpp>
#include <wcs/utilits/side_comparison.hpp>

using namespace wcs;

TEST_CASE("SideComparison")
{
    SECTION("Buy")
    {
        CHECK(utilits::sideLess<Side::Buy>(Price { 1.1 }, Price { 1.2 }));
        CHECK_FALSE(utilits::sideLess<Side::Buy>(Price { 1.2 }, Price { 1.1 }));
    
        CHECK(utilits::sideLessEqual<Side::Buy>(Price { 1.1 }, Price { 1.1 }));
        CHECK(utilits::sideLessEqual<Side::Buy>(Price { 1.1 }, Price { 1.2 }));
        CHECK_FALSE(utilits::sideLess<Side::Buy>(Price { 1.2 }, Price { 1.1 }));
    }
    
    SECTION("Sell")
    {
        CHECK(utilits::sideLess<Side::Sell>(Price { 1.2 }, Price { 1.1 }));
        CHECK_FALSE(utilits::sideLess<Side::Sell>(Price { 1.1 }, Price { 1.2 }));
    
        CHECK(utilits::sideLessEqual<Side::Sell>(Price { 1.1 }, Price { 1.1 }));
        CHECK(utilits::sideLessEqual<Side::Sell>(Price { 1.2 }, Price { 1.1 }));
        CHECK_FALSE(utilits::sideLess<Side::Sell>(Price { 1.1 }, Price { 1.2 }));
    }
}
