/*****************************************************************************
 *
 * This file is part of Mapnik (c++ mapping toolkit)
 *
 * Copyright (C) 2017 Artem Pavlenko
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

#ifndef MAPNIK_CAIRO_RENDER_POLYGON_PATTERN_HPP
#define MAPNIK_CAIRO_RENDER_POLYGON_PATTERN_HPP

#include <mapnik/symbolizer.hpp>
#include <mapnik/vertex_processor.hpp>
#include <mapnik/vertex_converters.hpp>
#include <mapnik/renderer_common/pattern_alignment.hpp>
#include <mapnik/renderer_common/render_pattern.hpp>
#include <mapnik/renderer_common/apply_vertex_converter.hpp>
#include <mapnik/renderer_common/clipping_extent.hpp>
#include <mapnik/agg_rasterizer.hpp>
#include <mapnik/marker.hpp>
#include <mapnik/cairo/render_pattern.hpp>

namespace mapnik {

struct cairo_pattern_base
{
    mapnik::marker const& marker_;
    renderer_common const& common_;
    symbolizer_base const& sym_;
    mapnik::feature_impl const& feature_;
    proj_transform const& prj_trans_;

    agg::trans_affine geom_transform() const
    {
        agg::trans_affine tr;
        auto transform = get_optional<transform_type>(sym_, keys::geometry_transform);
        if (transform)
        {
            evaluate_transform(tr, feature_, common_.vars_, *transform, common_.scale_factor_);
        }
        return tr;
    }
};

template <typename VertexConverter>
struct cairo_polygon_pattern : cairo_pattern_base
{
    cairo_polygon_pattern(mapnik::marker const& marker,
                          renderer_common const& common,
                          symbolizer_base const& sym,
                          mapnik::feature_impl const& feature,
                          proj_transform const& prj_trans)
        : cairo_pattern_base{marker, common, sym, feature, prj_trans},
          clip_(get<value_bool, keys::clip>(sym_, feature_, common_.vars_)),
          clip_box_(clip_box()),
          tr_(geom_transform()),
          converter_(clip_box_, sym, common.t_, prj_trans, tr_,
                     feature, common.vars_, common.scale_factor_)
    {
        value_double simplify_tolerance = get<value_double, keys::simplify_tolerance>(sym, feature, common_.vars_);
        value_double smooth = get<value_double, keys::smooth>(sym, feature, common_.vars_);

        converter_.template set<affine_transform_tag>();
        if (simplify_tolerance > 0.0) converter_.template set<simplify_tag>();
        if (smooth > 0.0) converter_.template set<smooth_tag>();
    }

    box2d<double> clip_box() const
    {
        return prj_trans_.equal() ? clipping_extent(common_) :
            box2d<double>(-1, -1, common_.width_ + 1, common_.height_ + 1);
    }

    void render(cairo_fill_rule_t fill_rule, cairo_context & context)
    {
        value_double opacity = get<value_double, keys::opacity>(sym_, feature_, common_.vars_);
        agg::trans_affine image_tr = agg::trans_affine_scaling(common_.scale_factor_);
        auto image_transform = get_optional<transform_type>(sym_, keys::image_transform);
        if (image_transform)
        {
            evaluate_transform(image_tr, feature_, common_.vars_, *image_transform, common_.scale_factor_);
        }

        composite_mode_e comp_op = get<composite_mode_e, keys::comp_op>(sym_, feature_, common_.vars_);

        cairo_save_restore guard(context);
        context.set_operator(comp_op);

        cairo_common_pattern_process_visitor visitor(common_, sym_, feature_);
        cairo_surface_ptr surface(util::apply_visitor(visitor, this->marker_));

        coord<double, 2> offset(0, 0);

        cairo_rectangle_t pattern_surface_extent;
        if (cairo_recording_surface_get_extents(surface.get(), &pattern_surface_extent))
        {
            offset = pattern_offset(sym_, feature_, prj_trans_, common_,
                pattern_surface_extent.width, pattern_surface_extent.height);
        }

        cairo_pattern pattern(surface);
        pattern.set_extend(CAIRO_EXTEND_REPEAT);
        pattern.set_origin(-offset.x, -offset.y);
        context.set_pattern(pattern);

        using apply_vertex_converter_type = detail::apply_vertex_converter<VertexConverter, cairo_context>;
        using vertex_processor_type = geometry::vertex_processor<apply_vertex_converter_type>;
        apply_vertex_converter_type apply(converter_, context);
        mapnik::util::apply_visitor(vertex_processor_type(apply),feature_.get_geometry());
        // fill polygon
        context.set_fill_rule(fill_rule);
        context.fill();
    }

    const bool clip_;
    const box2d<double> clip_box_;
    const agg::trans_affine tr_;
    VertexConverter converter_;
};

} // namespace mapnik


#endif // MAPNIK_CAIRO_RENDER_POLYGON_PATTERN_HPP
