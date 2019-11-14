
#include "catch.hpp"
#include <mapnik/text/harfbuzz_shaper.hpp>
#include <mapnik/text/font_library.hpp>

TEST_CASE("shapers compile") {

    mapnik::text_line line(0,0);
    mapnik::text_itemizer itemizer;
    std::vector<double> width_map;
    double scale_factor = 1;
    mapnik::font_library fl;
    mapnik::freetype_engine::font_file_mapping_type font_file_mapping;
    mapnik::freetype_engine::font_memory_cache_type font_memory_cache;
    mapnik::shaper_cache s_cache;
    mapnik::harfbuzz_shaper::shape_text(line, s_cache ,itemizer,
                                width_map,
                                scale_factor);
}
