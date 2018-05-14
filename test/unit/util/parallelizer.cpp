#include "catch.hpp"

#include <mapnik/util/parallelizer.hpp>
#include <mapnik/load_map.hpp>
#include <mapnik/image_util.hpp>
#include <mapnik/agg_renderer.hpp>

#include <istream>

TEST_CASE("parallelizer") {

SECTION("two layers") {

    mapnik::Map map(400, 400);
    mapnik::load_map(map, "test/data/good_maps/parallelization-1.xml");
    map.zoom_all();

    REQUIRE(mapnik::parallelizer::is_parallelizable(map));

    mapnik::image_rgba8 parallel_img(map.width(), map.height());

    const double scale_factor = 1;
    const double scale_denom = 0;
    mapnik::parallelizer::render(map, parallel_img, scale_denom, scale_factor);

    mapnik::image_rgba8 img(map.width(), map.height());
    mapnik::agg_renderer<mapnik::image_rgba8> ren(map, img, scale_factor);
    ren.apply();

    CHECK(parallel_img.painted() == img.painted());
    CHECK(mapnik::compare(parallel_img, img) == 0);

    //mapnik::save_to_file(parallel_img, "/tmp/_p1.png", "png32");
    //mapnik::save_to_file(img, "/tmp/_p_orig.png", "png32");
}

SECTION("layer comp-op") {

    mapnik::Map map(400, 400);
    mapnik::load_map(map, "test/data/good_maps/parallelization-layer-comp-op.xml");
    map.zoom_all();

    REQUIRE(mapnik::parallelizer::is_parallelizable(map));

    mapnik::image_rgba8 parallel_img(map.width(), map.height());

    const double scale_factor = 1;
    const double scale_denom = 0;
    mapnik::parallelizer::render(map, parallel_img, scale_denom, scale_factor);

    mapnik::image_rgba8 img(map.width(), map.height());
    mapnik::agg_renderer<mapnik::image_rgba8> ren(map, img, scale_factor);
    ren.apply();

    CHECK(parallel_img.painted() == img.painted());
    CHECK(mapnik::compare(parallel_img, img) == 0);

    //mapnik::save_to_file(parallel_img, "/tmp/_p1.png", "png32");
    //mapnik::save_to_file(img, "/tmp/_p_orig.png", "png32");
}

SECTION("layer scale denominator filter") {

    mapnik::Map map(400, 400);
    mapnik::load_map(map, "test/data/good_maps/parallelization-layer-denominator-filter.xml");
    map.zoom_all();

    REQUIRE(mapnik::parallelizer::is_parallelizable(map));

    mapnik::image_rgba8 parallel_img(map.width(), map.height());

    const double scale_factor = 1;
    const double scale_denom = 0;
    mapnik::parallelizer::render(map, parallel_img, scale_denom, scale_factor);

    mapnik::image_rgba8 img(map.width(), map.height());
    mapnik::agg_renderer<mapnik::image_rgba8> ren(map, img, scale_factor);
    ren.apply();

    CHECK(parallel_img.painted() == img.painted());
    CHECK(mapnik::compare(parallel_img, img) == 0);

    //mapnik::save_to_file(parallel_img, "/tmp/_p1.png", "png32");
    //mapnik::save_to_file(img, "/tmp/_p_orig.png", "png32");
}
}
