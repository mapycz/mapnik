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
#include <mapnik/cairo/cairo_render_vector.hpp>
#include <mapnik/renderer_common/render_pattern.hpp>
#include <mapnik/vertex_converters.hpp>
#include <mapnik/vertex_processor.hpp>
#include <mapnik/marker.hpp>
#include <mapnik/marker_cache.hpp>
#include <mapnik/agg_rasterizer.hpp>
#include <mapnik/renderer_common/clipping_extent.hpp>
#include <mapnik/renderer_common/apply_vertex_converter.hpp>
#include <mapnik/renderer_common/pattern_alignment.hpp>

namespace mapnik
{

template <typename T>
void cairo_renderer<T>::process(polygon_pattern_symbolizer const& sym,
                                  mapnik::feature_impl & feature,
                                  proj_transform const& prj_trans)
{
    composite_mode_e comp_op = get<composite_mode_e, keys::comp_op>(sym, feature, common_.vars_);
    std::string filename = get<std::string, keys::file>(sym, feature, common_.vars_);
    value_bool clip = get<value_bool, keys::clip>(sym, feature, common_.vars_);
    value_double simplify_tolerance = get<value_double, keys::simplify_tolerance>(sym, feature, common_.vars_);
    value_double smooth = get<value_double, keys::smooth>(sym, feature, common_.vars_);
    value_double opacity = get<value_double, keys::opacity>(sym, feature, common_.vars_);

    cairo_save_restore guard(context_);
    context_.set_operator(comp_op);

    std::shared_ptr<mapnik::marker const> marker = mapnik::marker_cache::instance().find(filename,true);
    if (marker->is<mapnik::marker_null>()) return;

    mapnik::rasterizer ra;
    common_pattern_process_visitor<polygon_pattern_symbolizer, mapnik::rasterizer> visitor(ra, common_, sym, feature);
    image_rgba8 image(util::apply_visitor(visitor, *marker));
    cairo_pattern pattern(image, opacity);
    pattern.set_extend(CAIRO_EXTEND_REPEAT);
    box2d<double> clip_box = clipping_extent(common_);
    coord<unsigned, 2> offset(detail::offset(sym, feature, prj_trans, common_, clip_box));
    pattern.set_origin(offset.x, offset.y);
    context_.set_pattern(pattern);

    agg::trans_affine tr;
    auto geom_transform = get_optional<transform_type>(sym, keys::geometry_transform);
    if (geom_transform) { evaluate_transform(tr, feature, common_.vars_, *geom_transform, common_.scale_factor_); }
    using vertex_converter_type = vertex_converter<clip_poly_tag,
                                                   transform_tag,
                                                   affine_transform_tag,
                                                   simplify_tag,
                                                   smooth_tag>;

    vertex_converter_type converter(clip_box,sym,common_.t_,prj_trans,tr,feature,common_.vars_,common_.scale_factor_);
    if (prj_trans.equal() && clip) converter.set<clip_poly_tag>();
    converter.set<transform_tag>(); //always transform
    converter.set<affine_transform_tag>();
    if (simplify_tolerance > 0.0) converter.set<simplify_tag>(); // optional simplify converter
    if (smooth > 0.0) converter.set<smooth_tag>(); // optional smooth converter

    using apply_vertex_converter_type = detail::apply_vertex_converter<vertex_converter_type, cairo_context>;
    using vertex_processor_type = geometry::vertex_processor<apply_vertex_converter_type>;
    apply_vertex_converter_type apply(converter, context_);
    mapnik::util::apply_visitor(vertex_processor_type(apply),feature.get_geometry());
    // fill polygon
    context_.set_fill_rule(CAIRO_FILL_RULE_EVEN_ODD);
    context_.fill();
}

template void cairo_renderer<cairo_ptr>::process(polygon_pattern_symbolizer const&,
                                                 mapnik::feature_impl &,
                                                 proj_transform const&);

}

#endif // HAVE_CAIRO
