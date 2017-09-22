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

// mapnik
#include <mapnik/feature.hpp>
#include <mapnik/debug.hpp>
#include <mapnik/image_any.hpp>
#include <mapnik/agg_renderer.hpp>
#include <mapnik/agg_rasterizer.hpp>
#include <mapnik/agg_pattern_source.hpp>
#include <mapnik/marker.hpp>
#include <mapnik/marker_cache.hpp>
#include <mapnik/symbolizer.hpp>
#include <mapnik/vertex_converters.hpp>
#include <mapnik/vertex_processor.hpp>
#include <mapnik/util/noncopyable.hpp>
#include <mapnik/parse_path.hpp>
#include <mapnik/renderer_common/clipping_extent.hpp>
#include <mapnik/renderer_common/render_pattern.hpp>
#include <mapnik/renderer_common/apply_vertex_converter.hpp>


#pragma GCC diagnostic push
#include <mapnik/warning_ignore_agg.hpp>
#include "agg_basics.h"
#include "agg_pixfmt_rgba.h"
#include "agg_color_rgba.h"
#include "agg_rendering_buffer.h"
#include "agg_rasterizer_outline.h"
#include "agg_rasterizer_outline_aa.h"
#include "agg_scanline_u.h"
#include "agg_renderer_scanline.h"
#include "agg_pattern_filters_rgba.h"
#include "agg_span_allocator.h"
#include "agg_span_pattern_rgba.h"
#include "agg_renderer_outline_image.h"
#pragma GCC diagnostic pop

namespace mapnik {

template <typename T0, typename T1>
void  agg_renderer<T0,T1>::process(line_pattern_symbolizer const& sym,
                               mapnik::feature_impl & feature,
                               proj_transform const& prj_trans)
{


    std::string filename = get<std::string, keys::file>(sym, feature, common_.vars_);
    if (filename.empty()) return;
    ras_ptr->reset();
    if (gamma_method_ != GAMMA_POWER || gamma_ != 1.0)
    {
        ras_ptr->gamma(agg::gamma_power());
        gamma_method_ = GAMMA_POWER;
        gamma_ = 1.0;
    }

    using color = agg::rgba8;
    using order = agg::order_rgba;
    using blender_type = agg::comp_op_adaptor_rgba_pre<color, order>;
    using pattern_filter_type = agg::pattern_filter_bilinear_rgba8;
    using pattern_type = agg::line_image_pattern<pattern_filter_type>;
    using pixfmt_type = agg::pixfmt_custom_blend_rgba<blender_type, agg::rendering_buffer>;
    using renderer_base = agg::renderer_base<pixfmt_type>;
    using renderer_type = agg::renderer_outline_image<renderer_base, pattern_type>;
    using rasterizer_type = agg::rasterizer_outline_aa<renderer_type>;

    value_double opacity = get<value_double, keys::opacity>(sym, feature, common_.vars_);
    value_bool clip = get<value_bool, keys::clip>(sym, feature, common_.vars_);
    value_double offset = get<value_double, keys::offset>(sym, feature, common_.vars_);
    value_double simplify_tolerance = get<value_double, keys::simplify_tolerance>(sym, feature, common_.vars_);
    value_double smooth = get<value_double, keys::smooth>(sym, feature, common_.vars_);

    buffer_type & current_buffer = buffers_.top().get();
    agg::rendering_buffer buf(current_buffer.bytes(), current_buffer.width(),
                              current_buffer.height(), current_buffer.row_size());
    pixfmt_type pixf(buf);
    pixf.comp_op(static_cast<agg::comp_op_e>(get<composite_mode_e, keys::comp_op>(sym, feature, common_.vars_)));
    renderer_base ren_base(pixf);
    agg::pattern_filter_bilinear_rgba8 filter;

    std::shared_ptr<mapnik::marker const> marker = marker_cache::instance().find(filename, true);

    common_pattern_process_visitor<line_pattern_symbolizer, rasterizer> visitor(*ras_ptr, common_, sym, feature);
    image_rgba8 image(util::apply_visitor(visitor, *marker));

    pattern_source source(image, opacity);
    pattern_type pattern (filter,source);
    renderer_type ren(ren_base, pattern);
    double half_stroke = std::max(image.width() / 2.0, image.height() / 2.0);
    int rast_clip_padding = static_cast<int>(std::round(half_stroke));
    ren.clip_box(-rast_clip_padding,-rast_clip_padding,common_.width_+rast_clip_padding,common_.height_+rast_clip_padding);
    rasterizer_type ras(ren);

    agg::trans_affine tr;
    auto transform = get_optional<transform_type>(sym, keys::geometry_transform);
    if (transform) evaluate_transform(tr, feature, common_.vars_, *transform, common_.scale_factor_);

    box2d<double> clip_box = clipping_extent(common_);
    if (clip)
    {
        double padding = (double)(common_.query_extent_.width() / common_.width_);
        if (half_stroke > 1)
            padding *= half_stroke;
        if (std::fabs(offset) > 0)
            padding *= std::fabs(offset) * 1.2;
        padding *= common_.scale_factor_;
        clip_box.pad(padding);
    }
    using vertex_converter_type = vertex_converter<clip_line_tag, transform_tag,
                                                   affine_transform_tag,
                                                   simplify_tag,smooth_tag,
                                                   offset_transform_tag>;

    vertex_converter_type converter(clip_box,sym,common_.t_,prj_trans,tr,feature,common_.vars_,common_.scale_factor_);

    if (clip) converter.set<clip_line_tag>();
    converter.set<transform_tag>(); //always transform
    if (simplify_tolerance > 0.0) converter.set<simplify_tag>(); // optional simplify converter
    if (std::fabs(offset) > 0.0) converter.set<offset_transform_tag>(); // parallel offset
    converter.set<affine_transform_tag>(); // optional affine transform
    if (smooth > 0.0) converter.set<smooth_tag>(); // optional smooth converter

    using apply_vertex_converter_type = detail::apply_vertex_converter<vertex_converter_type, rasterizer_type>;
    using vertex_processor_type = geometry::vertex_processor<apply_vertex_converter_type>;
    apply_vertex_converter_type apply(converter, ras);
    mapnik::util::apply_visitor(vertex_processor_type(apply), feature.get_geometry());
}

template void agg_renderer<image_rgba8>::process(line_pattern_symbolizer const&,
                                              mapnik::feature_impl &,
                                              proj_transform const&);

}
