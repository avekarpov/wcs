#include <catch2/catch.hpp>

#include <wcs/entities/price.hpp>

using namespace wcs;

TEST_CASE("Price")
{
    CHECK(Price { 0.0 } == Price { 0.0 });
    CHECK(Price { 0.0 } == Price { 0.0 + std::numeric_limits<double>::epsilon() });
    CHECK(Price { 0.0 } == Price { 0.0 - std::numeric_limits<double>::epsilon() });
    CHECK_FALSE(Price { 0.0 } == Price { 0.1 });
    CHECK_FALSE(Price { 0.0 } == Price { 0.0 + 2 * std::numeric_limits<double>::epsilon() });
    CHECK_FALSE(Price { 0.0 } == Price { 0.0 - 2 * std::numeric_limits<double>::epsilon() });
    
    CHECK(Price { 0.0 } != Price { 0.1 });
    CHECK(Price { 0.0 } != Price { 0.0 + 2 * std::numeric_limits<double>::epsilon() });
    CHECK(Price { 0.0 } != Price { 0.0 - 2 * std::numeric_limits<double>::epsilon() });
    CHECK_FALSE(Price { 0.0 } != Price { 0.0 });
    CHECK_FALSE(Price { 0.0 } != Price { 0.0 + std::numeric_limits<double>::epsilon() });
    CHECK_FALSE(Price { 0.0 } != Price { 0.0 - std::numeric_limits<double>::epsilon() });
    
    CHECK(Price { 0.0 } < Price { 0.1 });
    CHECK(Price { 0.0 } < Price { 0.0 + 2 * std::numeric_limits<double>::epsilon() });
    CHECK_FALSE(Price { 0.0 } < Price { -0.1 });
    CHECK_FALSE(Price { 0.0 } < Price { 0.0 + std::numeric_limits<double>::epsilon() });
    CHECK_FALSE(Price { 0.0 } < Price { 0.0 - std::numeric_limits<double>::epsilon() });
    CHECK_FALSE(Price { 0.0 } < Price { 0.0 - 2 * std::numeric_limits<double>::epsilon() });
    
    CHECK(Price { 0.0 } > Price { -0.1 });
    CHECK(Price { 0.0 } > Price { 0.0 - 2 * std::numeric_limits<double>::epsilon() });
    CHECK_FALSE(Price { 0.0 } > Price { 0.1 });
    CHECK_FALSE(Price { 0.0 } > Price { 0.0 + std::numeric_limits<double>::epsilon() });
    CHECK_FALSE(Price { 0.0 } > Price { 0.0 - std::numeric_limits<double>::epsilon() });
    CHECK_FALSE(Price { 0.0 } > Price { 0.0 + 2 * std::numeric_limits<double>::epsilon() });
    
    CHECK(Price { 0.0 } <= Price { 0.0 });
    CHECK(Price { 0.0 } <= Price { 0.1 });
    CHECK(Price { 0.0 } <= Price { 0.0 - std::numeric_limits<double>::epsilon() });
    CHECK(Price { 0.0 } <= Price { 0.0 + 2 * std::numeric_limits<double>::epsilon() });
    CHECK_FALSE(Price { 0.0 } <= Price { -0.1 });
    CHECK_FALSE(Price { 0.0 } <= Price { 0.0 - 2 * std::numeric_limits<double>::epsilon() });
    
    CHECK(Price { 0.0 } >= Price { 0.0 });
    CHECK(Price { 0.0 } >= Price { -0.1 });
    CHECK(Price { 0.0 } >= Price { 0.0 + std::numeric_limits<double>::epsilon() });
    CHECK(Price { 0.0 } >= Price { 0.0 - 2 * std::numeric_limits<double>::epsilon() });
    CHECK_FALSE(Price { 0.0 } >= Price { 0.1 });
    CHECK_FALSE(Price { 0.0 } >= Price { 0.0 + 2 * std::numeric_limits<double>::epsilon() });
}
