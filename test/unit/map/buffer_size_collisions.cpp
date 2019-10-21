#include "catch.hpp"

#include <map>
#include <tuple>
#include <boost/optional/optional_io.hpp>

#include <mapnik/map.hpp>
#include <mapnik/layer.hpp>
#include <mapnik/load_map.hpp>

TEST_CASE("buffer-size-collisions")
{
    SECTION("set buffer-size-collisions correctly from XML style")
    {
        mapnik::Map map;
        mapnik::load_map(map, "test/data/good_maps/buffer-size-collisions-1.xml");

        const int buffer_size = 100;
        const int buffer_size_collisions = 200;

        CHECK(map.buffer_size() == buffer_size);
        REQUIRE(map.buffer_size_collisions());
        CHECK(*map.buffer_size_collisions() == buffer_size_collisions);

        const std::map<std::string, boost::optional<int>> buffer_sizes = {
            { "dot", boost::none },
            { "line", boost::none },
            { "line-pattern", boost::none },
            { "polygon", boost::none },
            { "polygon-pattern", boost::none },
            { "point", buffer_size_collisions },
            { "marker1", buffer_size_collisions },
            { "marker2", boost::none },
            { "marker3", buffer_size_collisions },
            { "marker4", buffer_size_collisions },
            { "text1", buffer_size_collisions },
            { "text2", buffer_size_collisions },
            { "shield1", buffer_size_collisions },
            { "shield2", buffer_size_collisions },
            { "raster", boost::none },
            { "building", boost::none },
            { "collision", buffer_size_collisions },
            { "debug", boost::none },
            { "mix", buffer_size_collisions },
            { "marker-override", 30 },
            { "text-override", 30 }
        };

        for (auto const& layer : map.layers())
        {
            CHECK(std::make_tuple(layer.name(), layer.buffer_size()) ==
                std::make_tuple(layer.name(), buffer_sizes.at(layer.name())));
        }
    }
}
