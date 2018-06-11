#include "catch.hpp"

#include <mapnik/map.hpp>
#include <mapnik/feature_type_style.hpp>
#include <mapnik/rule.hpp>

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

    SECTION("find style mutable")
    {
        mapnik::Map map(256, 256);

        {
            mapnik::feature_type_style style;
            map.insert_style("style", std::move(style));
        }

        {
            boost::optional<mapnik::feature_type_style &> style =
                map.find_style("style");
            REQUIRE(style);
            CHECK(style->get_rules().size() == 0);
            style->add_rule(mapnik::rule());
        }

        {
            boost::optional<mapnik::feature_type_style &> style =
                map.find_style("style");
            REQUIRE(style);
            CHECK(style->get_rules().size() == 1);
        }
    }

    SECTION("find style immutable")
    {
        mapnik::Map map(256, 256);

        {
            mapnik::feature_type_style style;
            map.insert_style("style", std::move(style));
        }

        {
            mapnik::Map const& map2 = map;
            boost::optional<mapnik::feature_type_style const&> style =
                map2.find_style("style");
            REQUIRE(style);
            CHECK(style->get_rules().size() == 0);
        }
    }
}
