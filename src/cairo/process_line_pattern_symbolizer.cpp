/*****************************************************************************
 *
 * This file is part of Mapnik (c++ mapping toolkit)
 *
 * Copyright (C) 2015 Artem Pavlenko
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

#if defined(HAVE_CAIRO)

// mapnik
#include <mapnik/feature.hpp>
#include <mapnik/proj_transform.hpp>
#include <mapnik/cairo/cairo_renderer.hpp>
#include <mapnik/cairo/render_pattern.hpp>
#include <mapnik/cairo/render_polygon_pattern.hpp>
#include <mapnik/vertex_converters.hpp>
#include <mapnik/vertex_processor.hpp>
#include <mapnik/marker.hpp>
#include <mapnik/marker_cache.hpp>
#include <mapnik/renderer_common/apply_vertex_converter.hpp>

namespace mapnik
{

namespace
{

template <typename... Converters>
using vertex_converter_type = vertex_converter<clip_line_tag,
                                               transform_tag,
                                               affine_transform_tag,
                                               simplify_tag,
                                               smooth_tag,
                                               offset_transform_tag,
                                               Converters...>;

struct warp_pattern : cairo_pattern_base
{
    using vc_type = vertex_converter_type<>;

    warp_pattern(mapnik::marker const& marker,
                 renderer_common const& common,
                 symbolizer_base const& sym,
                 mapnik::feature_impl const& feature,
                 proj_transform const& prj_trans)
        : cairo_pattern_base{marker, common, sym, feature, prj_trans},
          clip_(get<value_bool, keys::clip>(sym, feature, common.vars_)),
          offset_(get<value_double, keys::offset>(sym, feature, common.vars_)),
          clip_box_(clipping_extent(common)),
          tr_(geom_transform()),
          converter_(clip_box_, sym, common.t_, prj_trans, tr_,
                     feature, common.vars_, common.scale_factor_)
    {
        value_double offset = get<value_double, keys::offset>(sym, feature, common.vars_);
        value_double simplify_tolerance = get<value_double, keys::simplify_tolerance>(sym, feature, common.vars_);
        value_double smooth = get<value_double, keys::smooth>(sym, feature, common.vars_);

        if (std::fabs(offset) > 0.0) converter_.template set<offset_transform_tag>();
        converter_.template set<affine_transform_tag>();
        if (simplify_tolerance > 0.0) converter_.template set<simplify_tag>();
        if (smooth > 0.0) converter_.template set<smooth_tag>();
        if (clip_) converter_.template set<clip_line_tag>();
        converter_.template set<transform_tag>();
    }

    box2d<double> clip_box() const
    {
        box2d<double> clipping_extent = common_.query_extent_;
        if (clip_)
        {
            double pad_per_pixel = static_cast<double>(common_.query_extent_.width() / common_.width_);
            double pixels = std::ceil(std::max(marker_.width() / 2.0 + std::fabs(offset_),
                                              (std::fabs(offset_) * 5.0/* TODO: offset_converter_default_threshold */)));
            double padding = pad_per_pixel * pixels * common_.scale_factor_;

            clipping_extent.pad(padding);
        }
        return clipping_extent;
    }

    void render(cairo_context & context)
    {
        cairo_common_pattern_process_visitor visitor(common_, sym_, feature_);
        cairo_surface_ptr surface(util::apply_visitor(visitor, this->marker_));

        cairo_rectangle_t pattern_extent;
        if (!cairo_recording_surface_get_extents(
            surface.get(), &pattern_extent))
        {
            MAPNIK_LOG_ERROR() << "Extent of the recording cairo "
                "surface has not been set";
        }


        cairo_pattern pattern(surface);
        pattern.set_extend(CAIRO_EXTEND_REPEAT);
        pattern.set_filter(CAIRO_FILTER_BILINEAR);

        cairo_save_restore guard(context);

        composite_mode_e comp_op = get<composite_mode_e, keys::comp_op>(sym_, feature_, common_.vars_);
        context.set_operator(comp_op);
        context.set_line_width(pattern_extent.height);

        using rasterizer_type = line_pattern_rasterizer<cairo_context>;
        rasterizer_type ras(context, pattern,
            pattern_extent.width,
            pattern_extent.height);

        using apply_vertex_converter_type = detail::apply_vertex_converter<
            vc_type, rasterizer_type>;
        using vertex_processor_type = geometry::vertex_processor<apply_vertex_converter_type>;
        apply_vertex_converter_type apply(converter_, ras);
        mapnik::util::apply_visitor(vertex_processor_type(apply), feature_.get_geometry());
    }

    const bool clip_;
    const double offset_;
    const box2d<double> clip_box_;
    const agg::trans_affine tr_;
    vc_type converter_;
};

using repeat_pattern_base = cairo_polygon_pattern<vertex_converter_type<dash_tag,
                                                                        stroke_tag>>;

struct repeat_pattern : repeat_pattern_base
{
    using repeat_pattern_base::cairo_polygon_pattern;

    void render(cairo_context & context)
    {
        converter_.template set<transform_tag>();

        if (has_key(sym_, keys::stroke_dasharray))
        {
            converter_.template set<dash_tag>();
        }

        if (clip_) converter_.template set<clip_line_tag>();

        value_double offset = get<value_double, keys::offset>(sym_, feature_, common_.vars_);
        if (std::fabs(offset) > 0.0) converter_.template set<offset_transform_tag>();

        repeat_pattern_base::render(CAIRO_FILL_RULE_WINDING, context);
    }
};

}

template <typename T>
void cairo_renderer<T>::process(line_pattern_symbolizer const& sym,
                                feature_impl & feature,
                                proj_transform const& prj_trans)
{
    std::string filename = get<std::string, keys::file>(sym, feature, common_.vars_);
    std::shared_ptr<mapnik::marker const> marker = marker_cache::instance().find(filename, true);

    if (marker->is<mapnik::marker_null>())
    {
        return;
    }

    line_pattern_enum pattern = get<line_pattern_enum, keys::line_pattern>(sym, feature, common_.vars_);
    switch (pattern)
    {
        case LINE_PATTERN_WARP:
        {
            warp_pattern pattern(*marker, common_, sym, feature, prj_trans);
            pattern.render(context_);
            break;
        }
        case LINE_PATTERN_REPEAT:
        {
            repeat_pattern pattern(*marker, common_, sym, feature, prj_trans);
            pattern.render(context_);
            break;
        }
        case line_pattern_enum_MAX:
        default:
            MAPNIK_LOG_ERROR(process_line_pattern_symbolizer) << "Incorrect line-pattern value.";
    }
}

template void cairo_renderer<cairo_ptr>::process(line_pattern_symbolizer const&,
                                                 mapnik::feature_impl &,
                                                 proj_transform const&);

}

#endif // HAVE_CAIRO
