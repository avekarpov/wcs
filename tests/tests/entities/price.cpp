#include <catch2/catch.hpp>

#include <wcs/entities/price.hpp>

using namespace wcs;

TEST_CASE("Price")
{
    CHECK(Price { 1.0 } == Price { 1.0 });
    CHECK(Price { 1.0 } == Price { 1.0 + std::numeric_limits<double>::epsilon() });
    CHECK(Price { 1.0 } == Price { 1.0 - std::numeric_limits<double>::epsilon() });
    CHECK_FALSE(Price { 1.0 } == Price { 0.1 });
    CHECK_FALSE(Price { 1.0 } == Price { 1.0 + 2 * std::numeric_limits<double>::epsilon() });
    CHECK_FALSE(Price { 1.0 } == Price { 1.0 - 2 * std::numeric_limits<double>::epsilon() });
    
    CHECK(Price { 1.0 } != Price { 0.1 });
    CHECK(Price { 1.0 } != Price { 1.0 + 2 * std::numeric_limits<double>::epsilon() });
    CHECK(Price { 1.0 } != Price { 1.0 - 2 * std::numeric_limits<double>::epsilon() });
    CHECK_FALSE(Price { 1.0 } != Price { 1.0 });
    CHECK_FALSE(Price { 1.0 } != Price { 1.0 + std::numeric_limits<double>::epsilon() });
    CHECK_FALSE(Price { 1.0 } != Price { 1.0 - std::numeric_limits<double>::epsilon() });
    
    CHECK(Price { 1.0 } < Price { 1.1 });
    CHECK(Price { 1.0 } < Price { 1.0 + 2 * std::numeric_limits<double>::epsilon() });
    CHECK_FALSE(Price { 1.0 } < Price { 0.1 });
    CHECK_FALSE(Price { 1.0 } < Price { 1.0 + std::numeric_limits<double>::epsilon() });
    CHECK_FALSE(Price { 1.0 } < Price { 1.0 - std::numeric_limits<double>::epsilon() });
    CHECK_FALSE(Price { 1.0 } < Price { 1.0 - 2 * std::numeric_limits<double>::epsilon() });
    
    CHECK(Price { 1.0 } > Price { 0.1 });
    CHECK(Price { 1.0 } > Price { 1.0 - 2 * std::numeric_limits<double>::epsilon() });
    CHECK_FALSE(Price { 1.0 } > Price { 2.0 });
    CHECK_FALSE(Price { 1.0 } > Price { 1.0 + std::numeric_limits<double>::epsilon() });
    CHECK_FALSE(Price { 1.0 } > Price { 1.0 - std::numeric_limits<double>::epsilon() });
    CHECK_FALSE(Price { 1.0 } > Price { 1.0 + 2 * std::numeric_limits<double>::epsilon() });
    
    CHECK(Price { 1.0 } <= Price { 1.0 });
    CHECK(Price { 1.0 } <= Price { 1.1 });
    CHECK(Price { 1.0 } <= Price { 1.0 - std::numeric_limits<double>::epsilon() });
    CHECK(Price { 1.0 } <= Price { 1.0 + 2 * std::numeric_limits<double>::epsilon() });
    CHECK_FALSE(Price { 1.0 } <= Price { 0.1 });
    CHECK_FALSE(Price { 1.0 } <= Price { 1.0 - 2 * std::numeric_limits<double>::epsilon() });
    
    CHECK(Price { 1.0 } >= Price { 1.0 });
    CHECK(Price { 1.0 } >= Price { 0.1 });
    CHECK(Price { 1.0 } >= Price { 1.0 + std::numeric_limits<double>::epsilon() });
    CHECK(Price { 1.0 } >= Price { 1.0 - 2 * std::numeric_limits<double>::epsilon() });
    CHECK_FALSE(Price { 1.0 } >= Price { 2.0 });
    CHECK_FALSE(Price { 1.0 } >= Price { 1.0 + 2 * std::numeric_limits<double>::epsilon() });
}
