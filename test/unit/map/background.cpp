#include "catch.hpp"

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

void render_agg(mapnik::Map const& map, mapnik::image_rgba8 & image)
{
    mapnik::agg_renderer<mapnik::image_rgba8> ren(map, image);
    ren.apply();
}

template <typename RenderFunction>
void test_background_blending(
    mapnik::color const& expected_color,
    mapnik::color const& background_color,
    mapnik::color const& original_color,
    boost::optional<mapnik::composite_mode_e> const& comp_op,
    RenderFunction render)
{
    const mapnik::image_rgba8 expected_image(
        solid_image(expected_color));

    mapnik::Map map(expected_image.width(), expected_image.height());
    map.set_background(background_color);
    if (comp_op)
    {
        map.set_background_comp_op(*comp_op);
    }

    mapnik::image_rgba8 actual_image(map.width(), map.height());
    mapnik::fill(actual_image, original_color);

    render(map, actual_image);

    CHECK(!actual_image.get_premultiplied());

    const std::size_t diff = mapnik::compare(actual_image,
        expected_image, 0, true);
    CHECK(diff == 0);
}

TEST_CASE("map background")
{
    SECTION("background blending: src_over (default)")
    {
        test_background_blending(
            mapnik::color(127, 128, 0),
            mapnik::color(0, 255, 0, 128),
            mapnik::color(255, 0, 0),
            boost::none,
            render_agg);
    }

    SECTION("background blending: src")
    {
        test_background_blending(
            mapnik::color(0, 255, 0, 128),
            mapnik::color(0, 255, 0, 128),
            mapnik::color(255, 0, 0),
            mapnik::composite_mode_e::src,
            render_agg);
    }

    SECTION("background blending: multiply")
    {
        test_background_blending(
            mapnik::color(127, 0, 0),
            mapnik::color(0, 255, 0, 128),
            mapnik::color(255, 0, 0),
            mapnik::composite_mode_e::multiply,
            render_agg);
    }
}
