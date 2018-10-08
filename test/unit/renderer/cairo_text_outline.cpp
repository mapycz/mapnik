#if defined(HAVE_CAIRO)
#include "catch.hpp"

#include <mapnik/map.hpp>
#include <mapnik/load_map.hpp>
#include <mapnik/cairo/cairo_renderer.hpp>

#include <sstream>

#include <cairo-pdf.h>

static cairo_status_t write(
	void *closure,
    const unsigned char *data,
    unsigned int length)
{
	std::ostream & ss = *static_cast<std::ostream*>(closure);
	ss.write(reinterpret_cast<char const *>(data), length);
    return ss ? CAIRO_STATUS_SUCCESS : CAIRO_STATUS_WRITE_ERROR;
}

static void render(std::ostream & output, bool text_outlines)
{
    const std::string map_style = R"STYLE(
<Map>
    <Style name="1">
        <Rule>
            <TextSymbolizer
                placement="point"
                face-name="DejaVu Sans Book"
                >
                "Text"
            </TextSymbolizer>
        </Rule>
    </Style>
    <Layer name="1">
        <StyleName>1</StyleName>
        <Datasource>
            <Parameter name="type">csv</Parameter>
            <Parameter name="inline">
			x, y
			0, 0
			</Parameter>
        </Datasource>
    </Layer>
</Map>
    )STYLE";

	mapnik::Map map(256, 256);
	mapnik::load_map_string(map, map_style);

    const std::string fontdir("fonts");
    REQUIRE(map.register_fonts(fontdir, true));

    const mapnik::box2d<double> bbox(-1, -1, 1, 1);
	map.zoom_to_box(bbox);

    mapnik::cairo_surface_ptr surface(
        // It would be better to use script surface,
        // but it's not included in the Mason Cairo library.
        cairo_pdf_surface_create_for_stream(
            write, &output, map.width(), map.height()),
        mapnik::cairo_surface_closer());
    mapnik::cairo_ptr image_context(
        mapnik::create_context(surface));
    mapnik::cairo_renderer<mapnik::cairo_ptr> ren(
        map, image_context, 1.0, 0, 0, text_outlines);
    ren.apply();
    cairo_surface_finish(&*surface);
}

TEST_CASE("cairo_renderer") {

SECTION("text_outlines") {

    {
        std::stringstream output;
        render(output, true);

        // Output should not contain any fonts
        CHECK(output.str().find("/Font") == std::string::npos);
    }

    {
        std::stringstream output;
        render(output, false);

        // Check fonts are there if text_outlines == false
        CHECK(output.str().find("/Font") != std::string::npos);
    }
}

}

#endif
