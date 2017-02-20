/*****************************************************************************
 *
 * This file is part of Mapnik (c++ mapping toolkit)
 *
 * Copyright (C) 2016 Artem Pavlenko
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 *
 *****************************************************************************/

#include <mapnik/label_collision_detector.hpp>
#include <mapnik/svg/svg_storage.hpp>
#include <mapnik/svg/svg_path_adapter.hpp>
#include <mapnik/marker_cache.hpp>
#include <mapnik/marker_helpers.hpp>
#include <mapnik/geometry_type.hpp>
#include <mapnik/renderer_common/render_markers_symbolizer.hpp>
#include <mapnik/symbolizer.hpp>

#include <mapnik/marker_layout.hpp>
#include <mapnik/text/line_layout.hpp>
#include <mapnik/text/grid_layout.hpp>
#include <mapnik/label_placement.hpp>
#include <mapnik/marker_grid_layout.hpp>
#include <mapnik/marker_line_layout.hpp>

#include <mapnik/grid_vertex_adapter.hpp>
#include <mapnik/label_placements/vertex_converter.hpp>
#include <mapnik/label_placements/vertex_first_layout.hpp>
#include <mapnik/label_placements/vertex_last_layout.hpp>
#include <mapnik/label_placements/vertex_layout.hpp>
#include <mapnik/label_placements/split_multi.hpp>
#include <mapnik/label_placements/geom_iterator.hpp>
#include <mapnik/label_placements/point_layout.hpp>
#include <mapnik/label_placements/point_geometry_visitor.hpp>
#include <mapnik/label_placements/interior_geometry_visitor.hpp>

namespace mapnik {

namespace label_placement {

struct marker_symbolizer_traits
{
    using point = split_multi<
        point_layout<point_geometry_visitor,
            geom_iterator<
                marker_layout>>>;
    using interior = split_multi<
        point_layout<interior_geometry_visitor,
            geom_iterator<
                marker_layout>>>;
    using vertex = split_multi<
        geom_iterator<
            vertex_layout<
                marker_layout>>>;
    using grid = split_multi<
        geom_iterator<
            vertex_converter<
                set_clip_geometry_visitor,
                marker_grid_layout<
                    geometry::grid_vertex_adapter<
                        geometry::spiral_grid_iterator>,
                    marker_layout>>>>;
    using alternating_grid = split_multi<
        geom_iterator<
            vertex_converter<
                set_clip_geometry_visitor,
                marker_grid_layout<
                    geometry::alternating_grid_vertex_adapter<
                        geometry::spiral_grid_iterator>,
                    marker_layout>>>>;
    using line = split_multi<
        geom_iterator<
            vertex_converter<
                set_line_clip_geometry_visitor,
                marker_line_layout<marker_layout>>>>;
    using vertex_first = split_multi<
        geom_iterator<
            vertex_converter<
                set_clip_geometry_visitor,
                vertex_first_layout<marker_layout>>>>;
    using vertex_last = split_multi<
        geom_iterator<
            vertex_converter<
                set_clip_geometry_visitor,
                vertex_last_layout<marker_layout>>>>;

    using placements_type = marker_positions_type;
    using layout_generator_type = marker_layout_generator;
};

}

namespace detail {

template <typename Detector, typename RendererType, typename ContextType>
struct render_marker_symbolizer_visitor
{
    render_marker_symbolizer_visitor(std::string const& filename,
                                     symbolizer_base const& sym,
                                     mapnik::feature_impl & feature,
                                     proj_transform const& prj_trans,
                                     RendererType & common,
                                     box2d<double> const& clip_box,
                                     ContextType & renderer_context)
        : filename_(filename),
          sym_(sym),
          feature_(feature),
          prj_trans_(prj_trans),
          common_(common),
          clip_box_(clip_box),
          renderer_context_(renderer_context) {}

    svg_attribute_type const& get_marker_attributes(svg_path_ptr const& stock_marker,
                                                    svg_attribute_type & custom_attr) const
    {
        auto const& stock_attr = stock_marker->attributes();
        if (push_explicit_style(stock_attr, custom_attr, sym_, feature_, common_.vars_))
            return custom_attr;
        else
            return stock_attr;
    }

    void operator() (marker_null const&) const {}

    void operator() (marker_svg const& mark)
    {
        using namespace mapnik::svg;

        // https://github.com/mapnik/mapnik/issues/1316
        bool snap_to_pixels = !mapnik::marker_cache::instance().is_uri(filename_);

        agg::trans_affine image_tr = agg::trans_affine_scaling(common_.scale_factor_);

        boost::optional<svg_path_ptr> const& stock_vector_marker = mark.get_data();
        svg_path_ptr marker_ptr = *stock_vector_marker;
        bool is_ellipse = false;

        svg_attribute_type s_attributes;
        auto const& r_attributes = get_marker_attributes(*stock_vector_marker, s_attributes);

        // special case for simple ellipse markers
        // to allow for full control over rx/ry dimensions
        if (filename_ == "shape://ellipse"
           && (has_key(sym_,keys::width) || has_key(sym_,keys::height)))
        {
            marker_ptr = std::make_shared<svg_storage_type>();
            is_ellipse = true;
        }
        else
        {
            box2d<double> const& bbox = mark.bounding_box();
            setup_transform_scaling(image_tr, bbox.width(), bbox.height(), feature_, common_.vars_, sym_);
        }

        vertex_stl_adapter<svg_path_storage> stl_storage(marker_ptr->source());
        svg_path_adapter svg_path(stl_storage);

        if (is_ellipse)
        {
            build_ellipse(sym_, feature_, common_.vars_, *marker_ptr, svg_path);
        }

        if (auto image_transform = get_optional<transform_type>(sym_, keys::image_transform))
        {
            evaluate_transform(image_tr, feature_, common_.vars_, *image_transform, common_.scale_factor_);
        }

        agg::trans_affine tr;
        auto transform = get_optional<transform_type>(sym_, keys::geometry_transform);
        if (transform) evaluate_transform(tr, feature_, common_.vars_, *transform, common_.scale_factor_);

        const label_placement::placement_params params {
            prj_trans_, common_.t_, tr, sym_, feature_, common_.vars_,
            box2d<double>(0, 0, common_.width_, common_.height_),
            common_.query_extent_, common_.scale_factor_, common_.symbol_cache_ };
        const auto placement_method = params.get<label_placement_enum, keys::label_placement>();

        using traits = label_placement::marker_symbolizer_traits;

        const box2d<double> marker_box(marker_ptr->bounding_box());
        const coord2d marker_center(marker_box.center());
        const agg::trans_affine_translation recenter(-marker_center.x, -marker_center.y);
        const agg::trans_affine marker_trans = recenter * image_tr;

        marker_layout_generator layout_generator(params,
            *common_.detector_, marker_box, marker_trans);

        label_placement::finder<traits>::apply(placement_method,
            layout_generator, params);

        boost::optional<std::string> key(get_optional<std::string>(
            sym_, keys::symbol_key, feature_, common_.vars_));

        for (auto const & placement : layout_generator.placements_)
        {
            agg::trans_affine matrix = marker_trans;
            matrix.rotate(placement.angle);
            matrix.translate(placement.pos.x, placement.pos.y);

            const markers_dispatch_params p(box2d<double>(), marker_trans,
                sym_, feature_, common_.vars_, common_.scale_factor_, snap_to_pixels);

            renderer_context_.render_marker(marker_ptr, svg_path, r_attributes, p, matrix);
            if (key)
            {
                common_.symbol_cache_.insert(*key, placement.box);
            }
        }
    }

    void operator() (marker_rgba8 const& mark)
    {
        agg::trans_affine image_tr = agg::trans_affine_scaling(common_.scale_factor_);

        setup_transform_scaling(image_tr, mark.width(), mark.height(), feature_, common_.vars_, sym_);
        auto image_transform = get_optional<transform_type>(sym_, keys::image_transform);
        if (image_transform) evaluate_transform(image_tr, feature_, common_.vars_, *image_transform, common_.scale_factor_);
        box2d<double> const& marker_box = mark.bounding_box();
        mapnik::image_rgba8 const& marker = mark.get_data();
        // - clamp sizes to > 4 pixels of interactivity
        coord2d marker_center = marker_box.center();

        agg::trans_affine tr;
        auto transform = get_optional<transform_type>(sym_, keys::geometry_transform);
        if (transform) evaluate_transform(tr, feature_, common_.vars_, *transform, common_.scale_factor_);

        const label_placement::placement_params params {
            prj_trans_, common_.t_, tr, sym_, feature_, common_.vars_,
            box2d<double>(0, 0, common_.width_, common_.height_),
            common_.query_extent_, common_.scale_factor_, common_.symbol_cache_ };
        const auto placement_method = params.get<label_placement_enum, keys::label_placement>();

        using traits = label_placement::marker_symbolizer_traits;

        const agg::trans_affine_translation recenter(-marker_center.x, -marker_center.y);
        const agg::trans_affine marker_trans = recenter * image_tr;

        marker_layout_generator layout_generator(params,
            *common_.detector_, marker_box, marker_trans);

        label_placement::finder<traits>::apply(placement_method,
            layout_generator, params);

        boost::optional<std::string> key(get_optional<std::string>(
            sym_, keys::symbol_key, feature_, common_.vars_));

        for (auto const & placement : layout_generator.placements_)
        {
            agg::trans_affine matrix = marker_trans;
            matrix.rotate(placement.angle);
            matrix.translate(placement.pos.x, placement.pos.y);

            const markers_dispatch_params p(box2d<double>(), marker_trans,
                sym_, feature_, common_.vars_, common_.scale_factor_, false);

            renderer_context_.render_marker(marker, p, matrix);
            if (key)
            {
                common_.symbol_cache_.insert(*key, placement.box);
            }
        }
    }

  private:
    std::string const& filename_;
    symbolizer_base const& sym_;
    mapnik::feature_impl & feature_;
    proj_transform const& prj_trans_;
    RendererType & common_;
    box2d<double> const& clip_box_;
    ContextType & renderer_context_;
};

} // namespace detail

markers_dispatch_params::markers_dispatch_params(box2d<double> const& size,
                                                 agg::trans_affine const& tr,
                                                 symbolizer_base const& sym,
                                                 feature_impl const& feature,
                                                 attributes const& vars,
                                                 double scale,
                                                 bool snap)
    : ignore_placement(get<value_bool, keys::ignore_placement>(sym, feature, vars))
    , key(get_optional<std::string>(sym, keys::symbol_key, feature, vars))
    , snap_to_pixels(snap)
    , scale_factor(scale)
    , opacity(get<value_double, keys::opacity>(sym, feature, vars))
{
}

void render_marker(
    symbolizer_base const& sym,
    mapnik::feature_impl & feature,
    proj_transform const& prj_trans,
    renderer_common & common,
    box2d<double> const& clip_box,
    markers_renderer_context & renderer_context,
    std::string const & filename,
    std::shared_ptr<marker const> const & mark)
{
    using Detector = label_collision_detector4;
    using RendererType = renderer_common;
    using ContextType = markers_renderer_context;
    using VisitorType = detail::render_marker_symbolizer_visitor<Detector,
                                                                 RendererType,
                                                                 ContextType>;

    VisitorType visitor(filename, sym, feature, prj_trans, common, clip_box,
                        renderer_context);
    util::apply_visitor(visitor, *mark);
}

void render_markers_symbolizer(markers_symbolizer const& sym,
                               mapnik::feature_impl & feature,
                               proj_transform const& prj_trans,
                               renderer_common & common,
                               box2d<double> const& clip_box,
                               markers_renderer_context & renderer_context)
{
    std::string filename = get<std::string>(sym, keys::file, feature, common.vars_, "shape://ellipse");
    if (!filename.empty())
    {
        auto mark = mapnik::marker_cache::instance().find(filename, true);
        render_marker(sym, feature, prj_trans, common, clip_box,
            renderer_context, filename, mark);
    }
}

void render_markers_symbolizer(point_symbolizer const& sym,
                               mapnik::feature_impl & feature,
                               proj_transform const& prj_trans,
                               renderer_common & common,
                               box2d<double> const& clip_box,
                               markers_renderer_context & renderer_context)
{
    std::string filename = get<std::string,keys::file>(sym,feature, common.vars_);
    std::shared_ptr<mapnik::marker const> mark = filename.empty()
       ? std::make_shared<mapnik::marker const>(mapnik::marker_rgba8())
       : marker_cache::instance().find(filename, true);
    render_marker(sym, feature, prj_trans, common, clip_box,
        renderer_context, filename, mark);
}

} // namespace mapnik
