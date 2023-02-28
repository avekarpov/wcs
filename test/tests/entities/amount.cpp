#include <catch2/catch.hpp>

#include <wcs/entities/amount.hpp>

using namespace wcs;

TEST_CASE("Amount")
{
    CHECK(Amount<Side::Buy> { 0.0 } == Amount<Side::Buy> { 0.0 });
    CHECK(Amount<Side::Buy> { 0.0 } == Amount<Side::Buy> { 0.0 + std::numeric_limits<double>::epsilon() });
    CHECK(Amount<Side::Buy> { 0.0 } == Amount<Side::Buy> { 0.0 - std::numeric_limits<double>::epsilon() });
    CHECK_FALSE(Amount<Side::Buy> { 0.0 } == Amount<Side::Buy> { 0.1 });
    CHECK_FALSE(Amount<Side::Buy> { 0.0 } == Amount<Side::Buy> { 0.0 + 2 * std::numeric_limits<double>::epsilon() });
    CHECK_FALSE(Amount<Side::Buy> { 0.0 } == Amount<Side::Buy> { 0.0 - 2 * std::numeric_limits<double>::epsilon() });
    
    CHECK(Amount<Side::Buy> { 0.0 } != Amount<Side::Buy> { 0.1 });
    CHECK(Amount<Side::Buy> { 0.0 } != Amount<Side::Buy> { 0.0 + 2 * std::numeric_limits<double>::epsilon() });
    CHECK(Amount<Side::Buy> { 0.0 } != Amount<Side::Buy> { 0.0 - 2 * std::numeric_limits<double>::epsilon() });
    CHECK_FALSE(Amount<Side::Buy> { 0.0 } != Amount<Side::Buy> { 0.0 });
    CHECK_FALSE(Amount<Side::Buy> { 0.0 } != Amount<Side::Buy> { 0.0 + std::numeric_limits<double>::epsilon() });
    CHECK_FALSE(Amount<Side::Buy> { 0.0 } != Amount<Side::Buy> { 0.0 - std::numeric_limits<double>::epsilon() });
    
    CHECK(Amount<Side::Buy> { 0.0 } < Amount<Side::Buy> { 0.1 });
    CHECK(Amount<Side::Buy> { 0.0 } < Amount<Side::Buy> { 0.0 + 2 * std::numeric_limits<double>::epsilon() });
    CHECK_FALSE(Amount<Side::Buy> { 0.0 } < Amount<Side::Buy> { -0.1 });
    CHECK_FALSE(Amount<Side::Buy> { 0.0 } < Amount<Side::Buy> { 0.0 + std::numeric_limits<double>::epsilon() });
    CHECK_FALSE(Amount<Side::Buy> { 0.0 } < Amount<Side::Buy> { 0.0 - std::numeric_limits<double>::epsilon() });
    CHECK_FALSE(Amount<Side::Buy> { 0.0 } < Amount<Side::Buy> { 0.0 - 2 * std::numeric_limits<double>::epsilon() });
    
    CHECK(Amount<Side::Buy> { 0.0 } > Amount<Side::Buy> { -0.1 });
    CHECK(Amount<Side::Buy> { 0.0 } > Amount<Side::Buy> { 0.0 - 2 * std::numeric_limits<double>::epsilon() });
    CHECK_FALSE(Amount<Side::Buy> { 0.0 } > Amount<Side::Buy> { 0.1 });
    CHECK_FALSE(Amount<Side::Buy> { 0.0 } > Amount<Side::Buy> { 0.0 + std::numeric_limits<double>::epsilon() });
    CHECK_FALSE(Amount<Side::Buy> { 0.0 } > Amount<Side::Buy> { 0.0 - std::numeric_limits<double>::epsilon() });
    CHECK_FALSE(Amount<Side::Buy> { 0.0 } > Amount<Side::Buy> { 0.0 + 2 * std::numeric_limits<double>::epsilon() });
    
    CHECK(Amount<Side::Buy> { 0.0 } <= Amount<Side::Buy> { 0.0 });
    CHECK(Amount<Side::Buy> { 0.0 } <= Amount<Side::Buy> { 0.1 });
    CHECK(Amount<Side::Buy> { 0.0 } <= Amount<Side::Buy> { 0.0 - std::numeric_limits<double>::epsilon() });
    CHECK(Amount<Side::Buy> { 0.0 } <= Amount<Side::Buy> { 0.0 + 2 * std::numeric_limits<double>::epsilon() });
    CHECK_FALSE(Amount<Side::Buy> { 0.0 } <= Amount<Side::Buy> { -0.1 });
    CHECK_FALSE(Amount<Side::Buy> { 0.0 } <= Amount<Side::Buy> { 0.0 - 2 * std::numeric_limits<double>::epsilon() });
    
    CHECK(Amount<Side::Buy> { 0.0 } >= Amount<Side::Buy> { 0.0 });
    CHECK(Amount<Side::Buy> { 0.0 } >= Amount<Side::Buy> { -0.1 });
    CHECK(Amount<Side::Buy> { 0.0 } >= Amount<Side::Buy> { 0.0 + std::numeric_limits<double>::epsilon() });
    CHECK(Amount<Side::Buy> { 0.0 } >= Amount<Side::Buy> { 0.0 - 2 * std::numeric_limits<double>::epsilon() });
    CHECK_FALSE(Amount<Side::Buy> { 0.0 } >= Amount<Side::Buy> { 0.1 });
    CHECK_FALSE(Amount<Side::Buy> { 0.0 } >= Amount<Side::Buy> { 0.0 + 2 * std::numeric_limits<double>::epsilon() });
}
