#include "catch.hpp"

#include <set>

#include <mapnik/map.hpp>
#include <mapnik/load_map.hpp>
#include <mapnik/rule.hpp>
#include <mapnik/feature_type_style.hpp>
#include <mapnik/symbolizer.hpp>
#include <mapnik/attribute_collector.hpp>

TEST_CASE("filter-factor")
{
    SECTION("default filter factor for bilinear-fast should be 1")
    {
        mapnik::Map map;
        mapnik::load_map(map, "test/data/good_maps/"
            "raster_symbolizer_filter_factor_scaling_bilinear_fast.xml");

        boost::optional<mapnik::feature_type_style const&> raster_style =
            const_cast<mapnik::Map const&>(map).find_style("raster");

        REQUIRE(raster_style);

        mapnik::rules const& rules = raster_style->get_rules();

        REQUIRE(rules.size() == 1);

        mapnik::rule const& rule = rules.front();
        mapnik::rule::symbolizers const& syms = rule.get_symbolizers();

        REQUIRE(syms.size() == 1);

        mapnik::symbolizer const& sym = syms.front();

        REQUIRE(sym.is<mapnik::raster_symbolizer>());

        mapnik::raster_symbolizer const& raster_sym =
            sym.get<mapnik::raster_symbolizer>();

        boost::optional<mapnik::value_double> filter_factor =
            mapnik::get_optional<mapnik::value_double>(
                raster_sym, mapnik::keys::filter_factor);

        REQUIRE(!filter_factor);

        std::set<std::string> names;
        mapnik::attribute_collector collector(names);
        collector(rule);

        CHECK(collector.get_filter_factor() == 1.0);
    }
}
