#include "catch.hpp"

#include <mapnik/util/parallelizer.hpp>
#include <mapnik/load_map.hpp>
#include <mapnik/image_util.hpp>
#include <mapnik/agg_renderer.hpp>
#include <mapnik/image_scaling.hpp>

#include <istream>

static void test_scale_denom(double scale_denom, bool should_pass)
{
    mapnik::Map map(1000, 1000);
    mapnik::load_map(map, "test/data/good_maps/parallelization-layer-scale-factor.xml");
    map.zoom_all();

    REQUIRE(mapnik::parallelizer::is_parallelizable(map));

    mapnik::image_rgba8 parallel_img(map.width(), map.height());

    const double scale_factor = 4;
    scale_denom /= scale_factor;
    mapnik::parallelizer::render(map, parallel_img, scale_denom, scale_factor);

    map.resize(map.width() / 4, map.height() / 4);
    mapnik::image_rgba8 img(map.width(), map.height());
    mapnik::agg_renderer<mapnik::image_rgba8> ren(map, img, 1);
    ren.apply();

    mapnik::image_rgba8 scaled_img(
        parallel_img.width(),
        parallel_img.height());
    mapnik::scale_image_agg(
        scaled_img, img,
        mapnik::SCALING_BILINEAR_FAST,
        4, 4, 0, 0, 1);

    CHECK(parallel_img.painted() == img.painted());
    CHECK((mapnik::compare(parallel_img, scaled_img) == 0) == should_pass);

    //mapnik::save_to_file(parallel_img, "/tmp/_p2.png", "png32");
    //mapnik::save_to_file(scaled_img, "/tmp/_p2_orig.png", "png32");
}

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

SECTION("layer with scale factor") {

    test_scale_denom(0, false);
    test_scale_denom(999, false);
    test_scale_denom(1000, true);
    test_scale_denom(5000, true);
    test_scale_denom(10000, true);
    test_scale_denom(10001, false);
    test_scale_denom(100000, false);
}

}
