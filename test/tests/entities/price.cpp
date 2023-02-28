#include <catch2/catch.hpp>

#include <wcs/entities/price.hpp>

using namespace wcs;

TEST_CASE("Price")
{
    CHECK(Price<Side::Buy> { 0.0 } == Price<Side::Buy> { 0.0 });
    CHECK(Price<Side::Buy> { 0.0 } == Price<Side::Buy> { 0.0 + std::numeric_limits<double>::epsilon() });
    CHECK(Price<Side::Buy> { 0.0 } == Price<Side::Buy> { 0.0 - std::numeric_limits<double>::epsilon() });
    CHECK_FALSE(Price<Side::Buy> { 0.0 } == Price<Side::Buy> { 0.1 });
    CHECK_FALSE(Price<Side::Buy> { 0.0 } == Price<Side::Buy> { 0.0 + 2 * std::numeric_limits<double>::epsilon() });
    CHECK_FALSE(Price<Side::Buy> { 0.0 } == Price<Side::Buy> { 0.0 - 2 * std::numeric_limits<double>::epsilon() });
    
    CHECK(Price<Side::Buy> { 0.0 } != Price<Side::Buy> { 0.1 });
    CHECK(Price<Side::Buy> { 0.0 } != Price<Side::Buy> { 0.0 + 2 * std::numeric_limits<double>::epsilon() });
    CHECK(Price<Side::Buy> { 0.0 } != Price<Side::Buy> { 0.0 - 2 * std::numeric_limits<double>::epsilon() });
    CHECK_FALSE(Price<Side::Buy> { 0.0 } != Price<Side::Buy> { 0.0 });
    CHECK_FALSE(Price<Side::Buy> { 0.0 } != Price<Side::Buy> { 0.0 + std::numeric_limits<double>::epsilon() });
    CHECK_FALSE(Price<Side::Buy> { 0.0 } != Price<Side::Buy> { 0.0 - std::numeric_limits<double>::epsilon() });
    
    CHECK(Price<Side::Buy> { 0.0 } < Price<Side::Buy> { 0.1 });
    CHECK(Price<Side::Buy> { 0.0 } < Price<Side::Buy> { 0.0 + 2 * std::numeric_limits<double>::epsilon() });
    CHECK_FALSE(Price<Side::Buy> { 0.0 } < Price<Side::Buy> { -0.1 });
    CHECK_FALSE(Price<Side::Buy> { 0.0 } < Price<Side::Buy> { 0.0 + std::numeric_limits<double>::epsilon() });
    CHECK_FALSE(Price<Side::Buy> { 0.0 } < Price<Side::Buy> { 0.0 - std::numeric_limits<double>::epsilon() });
    CHECK_FALSE(Price<Side::Buy> { 0.0 } < Price<Side::Buy> { 0.0 - 2 * std::numeric_limits<double>::epsilon() });
    
    CHECK(Price<Side::Buy> { 0.0 } > Price<Side::Buy> { -0.1 });
    CHECK(Price<Side::Buy> { 0.0 } > Price<Side::Buy> { 0.0 - 2 * std::numeric_limits<double>::epsilon() });
    CHECK_FALSE(Price<Side::Buy> { 0.0 } > Price<Side::Buy> { 0.1 });
    CHECK_FALSE(Price<Side::Buy> { 0.0 } > Price<Side::Buy> { 0.0 + std::numeric_limits<double>::epsilon() });
    CHECK_FALSE(Price<Side::Buy> { 0.0 } > Price<Side::Buy> { 0.0 - std::numeric_limits<double>::epsilon() });
    CHECK_FALSE(Price<Side::Buy> { 0.0 } > Price<Side::Buy> { 0.0 + 2 * std::numeric_limits<double>::epsilon() });
    
    CHECK(Price<Side::Buy> { 0.0 } <= Price<Side::Buy> { 0.0 });
    CHECK(Price<Side::Buy> { 0.0 } <= Price<Side::Buy> { 0.1 });
    CHECK(Price<Side::Buy> { 0.0 } <= Price<Side::Buy> { 0.0 - std::numeric_limits<double>::epsilon() });
    CHECK(Price<Side::Buy> { 0.0 } <= Price<Side::Buy> { 0.0 + 2 * std::numeric_limits<double>::epsilon() });
    CHECK_FALSE(Price<Side::Buy> { 0.0 } <= Price<Side::Buy> { -0.1 });
    CHECK_FALSE(Price<Side::Buy> { 0.0 } <= Price<Side::Buy> { 0.0 - 2 * std::numeric_limits<double>::epsilon() });
    
    CHECK(Price<Side::Buy> { 0.0 } >= Price<Side::Buy> { 0.0 });
    CHECK(Price<Side::Buy> { 0.0 } >= Price<Side::Buy> { -0.1 });
    CHECK(Price<Side::Buy> { 0.0 } >= Price<Side::Buy> { 0.0 + std::numeric_limits<double>::epsilon() });
    CHECK(Price<Side::Buy> { 0.0 } >= Price<Side::Buy> { 0.0 - 2 * std::numeric_limits<double>::epsilon() });
    CHECK_FALSE(Price<Side::Buy> { 0.0 } >= Price<Side::Buy> { 0.1 });
    CHECK_FALSE(Price<Side::Buy> { 0.0 } >= Price<Side::Buy> { 0.0 + 2 * std::numeric_limits<double>::epsilon() });
}
