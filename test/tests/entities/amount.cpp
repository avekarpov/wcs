#include <catch2/catch.hpp>

#include <wcs/entities/amount.hpp>

using namespace wcs;

TEST_CASE("Amount")
{
    CHECK(Amount { 0.0 } == Amount { 0.0 });
    CHECK(Amount { 0.0 } == Amount { 0.0 + std::numeric_limits<double>::epsilon() });
    CHECK(Amount { 0.0 } == Amount { 0.0 - std::numeric_limits<double>::epsilon() });
    CHECK_FALSE(Amount { 0.0 } == Amount { 0.1 });
    CHECK_FALSE(Amount { 0.0 } == Amount { 0.0 + 2 * std::numeric_limits<double>::epsilon() });
    CHECK_FALSE(Amount { 0.0 } == Amount { 0.0 - 2 * std::numeric_limits<double>::epsilon() });
    
    CHECK(Amount { 0.0 } != Amount { 0.1 });
    CHECK(Amount { 0.0 } != Amount { 0.0 + 2 * std::numeric_limits<double>::epsilon() });
    CHECK(Amount { 0.0 } != Amount { 0.0 - 2 * std::numeric_limits<double>::epsilon() });
    CHECK_FALSE(Amount { 0.0 } != Amount { 0.0 });
    CHECK_FALSE(Amount { 0.0 } != Amount { 0.0 + std::numeric_limits<double>::epsilon() });
    CHECK_FALSE(Amount { 0.0 } != Amount { 0.0 - std::numeric_limits<double>::epsilon() });
    
    CHECK(Amount { 0.0 } < Amount { 0.1 });
    CHECK(Amount { 0.0 } < Amount { 0.0 + 2 * std::numeric_limits<double>::epsilon() });
    CHECK_FALSE(Amount { 0.0 } < Amount { -0.1 });
    CHECK_FALSE(Amount { 0.0 } < Amount { 0.0 + std::numeric_limits<double>::epsilon() });
    CHECK_FALSE(Amount { 0.0 } < Amount { 0.0 - std::numeric_limits<double>::epsilon() });
    CHECK_FALSE(Amount { 0.0 } < Amount { 0.0 - 2 * std::numeric_limits<double>::epsilon() });
    
    CHECK(Amount { 0.0 } > Amount { -0.1 });
    CHECK(Amount { 0.0 } > Amount { 0.0 - 2 * std::numeric_limits<double>::epsilon() });
    CHECK_FALSE(Amount { 0.0 } > Amount { 0.1 });
    CHECK_FALSE(Amount { 0.0 } > Amount { 0.0 + std::numeric_limits<double>::epsilon() });
    CHECK_FALSE(Amount { 0.0 } > Amount { 0.0 - std::numeric_limits<double>::epsilon() });
    CHECK_FALSE(Amount { 0.0 } > Amount { 0.0 + 2 * std::numeric_limits<double>::epsilon() });
    
    CHECK(Amount { 0.0 } <= Amount { 0.0 });
    CHECK(Amount { 0.0 } <= Amount { 0.1 });
    CHECK(Amount { 0.0 } <= Amount { 0.0 - std::numeric_limits<double>::epsilon() });
    CHECK(Amount { 0.0 } <= Amount { 0.0 + 2 * std::numeric_limits<double>::epsilon() });
    CHECK_FALSE(Amount { 0.0 } <= Amount { -0.1 });
    CHECK_FALSE(Amount { 0.0 } <= Amount { 0.0 - 2 * std::numeric_limits<double>::epsilon() });
    
    CHECK(Amount { 0.0 } >= Amount { 0.0 });
    CHECK(Amount { 0.0 } >= Amount { -0.1 });
    CHECK(Amount { 0.0 } >= Amount { 0.0 + std::numeric_limits<double>::epsilon() });
    CHECK(Amount { 0.0 } >= Amount { 0.0 - 2 * std::numeric_limits<double>::epsilon() });
    CHECK_FALSE(Amount { 0.0 } >= Amount { 0.1 });
    CHECK_FALSE(Amount { 0.0 } >= Amount { 0.0 + 2 * std::numeric_limits<double>::epsilon() });
}
