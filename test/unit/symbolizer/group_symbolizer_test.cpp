
#include "catch.hpp"

#include <iostream>
#include <mapnik/map.hpp>
#include <mapnik/layer.hpp>
#include <mapnik/rule.hpp>
#include <mapnik/feature_type_style.hpp>
#include <mapnik/symbolizer.hpp>
#include <mapnik/agg_renderer.hpp>
#include <mapnik/expression.hpp>
#include <mapnik/memory_datasource.hpp>
#include <mapnik/group/group_rule.hpp>
#include <mapnik/util/fs.hpp>

TEST_CASE("group symbolizer") {

SECTION("empty rule") {

    using namespace mapnik;

    try
    {
        Map m(256, 256);

        feature_type_style style;
        {
            rule r;
            group_symbolizer group_sym;
            group_symbolizer_properties_ptr prop =
                std::make_shared<group_symbolizer_properties>();

            expression_ptr repeat_key,
                filter(std::make_shared<mapnik::expr_node>(true));
            group_rule_ptr rule =
                std::make_shared<group_rule>(filter, repeat_key);
            prop->add_rule(std::move(rule));

            put(group_sym, keys::group_properties, prop);
            r.append(std::move(group_sym));
            style.add_rule(std::move(r));
        }
        m.insert_style("st", std::move(style));

        context_ptr context =
            std::make_shared<context_type>();
        feature_ptr feature =
            std::make_shared<feature_impl>(context, 0);
        feature->set_geometry(geometry::point<double>(0, 0));
        mapnik::parameters params;
        std::shared_ptr<memory_datasource> ds = std::make_shared<memory_datasource>(params);
        ds->push(feature);

        layer lyr("layer");
        lyr.set_datasource(ds);
        lyr.add_style("st");
        m.add_layer(lyr);

        m.zoom_all();

        image_rgba8 image(m.width(), m.height());
        agg_renderer<image_rgba8> ren(m, image);
        ren.apply();
    }
    catch (std::exception const & ex)
    {
        std::clog << ex.what() << std::endl;
        REQUIRE(false);
    }

}
}
