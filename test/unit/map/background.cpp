#include "catch.hpp"

#include <iostream>
#include <mapnik/map.hpp>
#include <mapnik/image_util.hpp>
#include <mapnik/agg_renderer.hpp>

mapnik::image_rgba8 solid_image(
    mapnik::color const& color)
{
    const std::size_t d = 10;
    mapnik::image_rgba8 image(d, d);
    mapnik::fill(image, color);
    return image;
}

TEST_CASE("map background")
{
    SECTION("background blending: src_over (default)")
    {
        const mapnik::color expected_color(127, 128, 0);
        const mapnik::image_rgba8 expected_image(
            solid_image(expected_color));

        mapnik::Map map(expected_image.width(), expected_image.height());
        map.set_background(mapnik::color(0, 255, 0, 128));

        mapnik::image_rgba8 actual_image(map.width(), map.height());
        const mapnik::color original_color(255, 0, 0);
        mapnik::fill(actual_image, original_color);

        mapnik::agg_renderer<mapnik::image_rgba8> ren(map, actual_image);
        ren.apply();

        CHECK(!actual_image.get_premultiplied());

        const std::size_t diff = mapnik::compare(actual_image,
            expected_image, 0, true);
        CHECK(diff == 0);
    }

    SECTION("background blending: src")
    {
        const mapnik::color expected_color(0, 255, 0, 128);
        const mapnik::image_rgba8 expected_image(
            solid_image(expected_color));

        mapnik::Map map(expected_image.width(), expected_image.height());
        map.set_background(mapnik::color(0, 255, 0, 128));
        map.set_background_comp_op(mapnik::composite_mode_e::src);

        mapnik::image_rgba8 actual_image(map.width(), map.height());
        const mapnik::color original_color(255, 0, 0);
        mapnik::fill(actual_image, original_color);

        mapnik::agg_renderer<mapnik::image_rgba8> ren(map, actual_image);
        ren.apply();

        CHECK(!actual_image.get_premultiplied());

        const std::size_t diff = mapnik::compare(actual_image,
            expected_image, 0, true);
        CHECK(diff == 0);
    }

    SECTION("background blending: multiply")
    {
        const mapnik::color expected_color(127, 0, 0);
        const mapnik::image_rgba8 expected_image(
            solid_image(expected_color));

        mapnik::Map map(expected_image.width(), expected_image.height());
        map.set_background(mapnik::color(0, 255, 0, 128));
        map.set_background_comp_op(mapnik::composite_mode_e::multiply);

        mapnik::image_rgba8 actual_image(map.width(), map.height());
        const mapnik::color original_color(255, 0, 0);
        mapnik::fill(actual_image, original_color);

        mapnik::agg_renderer<mapnik::image_rgba8> ren(map, actual_image);
        ren.apply();

        CHECK(!actual_image.get_premultiplied());
        std::clog << mapnik::get_pixel<mapnik::color>(actual_image, 0,
        0) << std::endl;

        const std::size_t diff = mapnik::compare(actual_image,
            expected_image, 0, true);
        CHECK(diff == 0);
    }
}
