#include "catch.hpp"

#include <mapnik/map.hpp>

TEST_CASE("map")
{
    SECTION("map resize")
    {
        mapnik::Map map(256, 256);

        CHECK(map.width() == 256);
        CHECK(map.height() == 256);

        map.resize(0, 0);

        CHECK(map.width() == 256);
        CHECK(map.height() == 256);

        map.resize(0, 1);

        CHECK(map.width() == 256);
        CHECK(map.height() == 256);

        map.resize(1, 0);

        CHECK(map.width() == 256);
        CHECK(map.height() == 256);

        map.resize(1, 1);

        CHECK(map.width() == 1);
        CHECK(map.height() == 1);

        map.resize(16, 16);

        CHECK(map.width() == 16);
        CHECK(map.height() == 16);

        map.resize(128000, 64000);

        CHECK(map.width() == 128000);
        CHECK(map.height() == 64000);

        map.resize(5000000, 5000000);

        CHECK(map.width() == 5000000);
        CHECK(map.height() == 5000000);
    }
}
