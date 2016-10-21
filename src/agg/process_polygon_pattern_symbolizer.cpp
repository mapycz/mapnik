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

// mapnik
#include <mapnik/feature.hpp>
#include <mapnik/debug.hpp>
#include <mapnik/agg_renderer.hpp>
#include <mapnik/agg_helpers.hpp>
#include <mapnik/agg_rasterizer.hpp>
#include <mapnik/marker.hpp>
#include <mapnik/marker_cache.hpp>
#include <mapnik/vertex_converters.hpp>
#include <mapnik/vertex_processor.hpp>
#include <mapnik/parse_path.hpp>
#include <mapnik/symbolizer.hpp>
#include <mapnik/svg/svg_converter.hpp>
#include <mapnik/svg/svg_renderer_agg.hpp>
#include <mapnik/svg/svg_path_adapter.hpp>
#include <mapnik/renderer_common/clipping_extent.hpp>
#include <mapnik/renderer_common/render_pattern.hpp>
#include <mapnik/renderer_common/apply_vertex_converter.hpp>
#include <mapnik/renderer_common/pattern_alignment.hpp>

#pragma GCC diagnostic push
#include <mapnik/warning_ignore_agg.hpp>
#include "agg_basics.h"
#include "agg_rendering_buffer.h"
#include "agg_pixfmt_rgba.h"
#include "agg_color_rgba.h"
#include "agg_rasterizer_scanline_aa.h"
#include "agg_scanline_u.h"
// for polygon_pattern_symbolizer
#include "agg_renderer_scanline.h"
#include "agg_span_allocator.h"
#include "agg_span_pattern_rgba.h"
#include "agg_image_accessors.h"
#include "agg_conv_clip_polygon.h"
#pragma GCC diagnostic pop

namespace mapnik {

template <typename T0, typename T1>
void agg_renderer<T0,T1>::process(polygon_pattern_symbolizer const& sym,
                                  mapnik::feature_impl & feature,
                                  proj_transform const& prj_trans)
{
    std::string filename = get<std::string, keys::file>(sym, feature, common_.vars_);
    if (filename.empty()) return;
    std::shared_ptr<mapnik::marker const> marker = marker_cache::instance().find(filename, true);

    agg::rendering_buffer buf(current_buffer_->bytes(), current_buffer_->width(),
                              current_buffer_->height(), current_buffer_->row_size());
    ras_ptr->reset();
    value_double gamma = get<value_double, keys::gamma>(sym, feature, common_.vars_);
    gamma_method_enum gamma_method = get<gamma_method_enum, keys::gamma_method>(sym, feature, common_.vars_);
    if (gamma != gamma_ || gamma_method != gamma_method_)
    {
        set_gamma_method(ras_ptr, gamma, gamma_method);
        gamma_method_ = gamma_method;
        gamma_ = gamma;
    }

    value_bool clip = get<value_bool, keys::clip>(sym, feature, common_.vars_);
    value_double opacity = get<double, keys::opacity>(sym, feature, common_.vars_);
    value_double simplify_tolerance = get<value_double, keys::simplify_tolerance>(sym, feature, common_.vars_);
    value_double smooth = get<value_double, keys::smooth>(sym, feature, common_.vars_);

    using color = agg::rgba8;
    using order = agg::order_rgba;
    using blender_type = agg::comp_op_adaptor_rgba_pre<color, order>;
    using pixfmt_type = agg::pixfmt_custom_blend_rgba<blender_type, agg::rendering_buffer>;

    using wrap_x_type = agg::wrap_mode_repeat;
    using wrap_y_type = agg::wrap_mode_repeat;
    using img_source_type = agg::image_accessor_wrap<agg::pixfmt_rgba32_pre,
                                                     wrap_x_type,
                                                     wrap_y_type>;

    using span_gen_type = agg::span_pattern_rgba<img_source_type>;
    using ren_base = agg::renderer_base<pixfmt_type>;

    using renderer_type = agg::renderer_scanline_aa_alpha<ren_base,
                                                          agg::span_allocator<agg::rgba8>,
                                                          span_gen_type>;

    pixfmt_type pixf(buf);
    pixf.comp_op(static_cast<agg::comp_op_e>(get<composite_mode_e, keys::comp_op>(sym, feature, common_.vars_)));
    ren_base renb(pixf);

    common_pattern_process_visitor<polygon_pattern_symbolizer, rasterizer> visitor(*ras_ptr, common_, sym, feature);
    image_rgba8 image(util::apply_visitor(visitor, *marker));

    unsigned w = image.width();
    unsigned h = image.height();
    agg::rendering_buffer pattern_rbuf((agg::int8u*)image.bytes(),w,h,w*4);
    agg::pixfmt_rgba32_pre pixf_pattern(pattern_rbuf);
    img_source_type img_src(pixf_pattern);

    unsigned offset_x=0;
    unsigned offset_y=0;
    box2d<double> clip_box = clipping_extent(common_);
    pattern_alignment_enum alignment = get<pattern_alignment_enum, keys::alignment>(sym, feature, common_.vars_);
    if (alignment == LOCAL_ALIGNMENT)
    {
        double x0 = 0;
        double y0 = 0;
        using apply_local_alignment = detail::apply_local_alignment;
        apply_local_alignment apply(common_.t_,prj_trans, clip_box, x0, y0);
        util::apply_visitor(geometry::vertex_processor<apply_local_alignment>(apply), feature.get_geometry());
        offset_x = std::abs(clip_box.width() - x0);
        offset_y = std::abs(clip_box.height() - y0);
    }

    span_gen_type sg(img_src, offset_x, offset_y);

    agg::span_allocator<agg::rgba8> sa;
    renderer_type rp(renb,sa, sg, unsigned(opacity * 255));

    agg::trans_affine tr;
    auto transform = get_optional<transform_type>(sym, keys::geometry_transform);
    if (transform) evaluate_transform(tr, feature, common_.vars_, *transform, common_.scale_factor_);
    using vertex_converter_type = vertex_converter<clip_poly_tag,
                                                   transform_tag,
                                                   affine_transform_tag,
                                                   simplify_tag,
                                                   smooth_tag>;

    vertex_converter_type converter(clip_box, sym,common_.t_,prj_trans,tr,feature,common_.vars_,common_.scale_factor_);

    if (prj_trans.equal() && clip) converter.set<clip_poly_tag>();
    converter.set<transform_tag>(); //always transform
    converter.set<affine_transform_tag>(); // optional affine transform
    if (simplify_tolerance > 0.0) converter.set<simplify_tag>(); // optional simplify converter
    if (smooth > 0.0) converter.set<smooth_tag>(); // optional smooth converter

    using apply_vertex_converter_type = detail::apply_vertex_converter<vertex_converter_type, rasterizer>;
    using vertex_processor_type = geometry::vertex_processor<apply_vertex_converter_type>;
    apply_vertex_converter_type apply(converter, *ras_ptr);
    mapnik::util::apply_visitor(vertex_processor_type(apply),feature.get_geometry());
    agg::scanline_u8 sl;
    ras_ptr->filling_rule(agg::fill_even_odd);
    agg::render_scanlines(*ras_ptr, sl, rp);
}

template void agg_renderer<image_rgba8>::process(polygon_pattern_symbolizer const&,
                                                 mapnik::feature_impl &,
                                                 proj_transform const&);

}
