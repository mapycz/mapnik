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
#include <mapnik/renderer_common/render_pattern.hpp>
#include <mapnik/box2d.hpp>
#include <mapnik/svg/svg_converter.hpp>
#include <mapnik/svg/svg_renderer_agg.hpp>
#include <mapnik/svg/svg_path_adapter.hpp>
#include <mapnik/agg_rasterizer.hpp>
#include <mapnik/renderer_common.hpp>
#include <mapnik/util/const_rendering_buffer.hpp>

#pragma GCC diagnostic push
#include <mapnik/warning_ignore_agg.hpp>
#include "agg_rendering_buffer.h"
#include "agg_pixfmt_rgba.h"
#include "agg_pixfmt_gray.h"
#include "agg_color_rgba.h"
#include "agg_color_gray.h"
#include "agg_scanline_u.h"
#include "agg_image_accessors.h"
#include "agg_span_image_filter_rgba.h"
#include "agg_rasterizer_scanline_aa.h"
#pragma GCC diagnostic pop

namespace mapnik {

image_rgba8 render_pattern(rasterizer & ras,
                           marker_svg const & marker,
                           box2d<double> const & bbox,
                           agg::trans_affine const & tr)
{
    using pixfmt = agg::pixfmt_rgba32_pre;
    using renderer_base = agg::renderer_base<pixfmt>;
    using renderer_solid = agg::renderer_scanline_aa_solid<renderer_base>;
    agg::scanline_u8 sl;

    image_rgba8 image(bbox.width(), bbox.height());
    agg::rendering_buffer buf(image.bytes(), image.width(), image.height(), image.row_size());
    pixfmt pixf(buf);
    renderer_base renb(pixf);

    svg::vertex_stl_adapter<svg::svg_path_storage> stl_storage(marker.get_data()->source());
    svg::svg_path_adapter svg_path(stl_storage);
    svg::svg_renderer_agg<svg::svg_path_adapter,
                                  agg::pod_bvector<svg::path_attributes>,
                                  renderer_solid,
                                  pixfmt > svg_renderer(svg_path,
                                                        marker.get_data()->attributes());

    svg_renderer.render(ras, sl, renb, tr, 1.0, bbox);
    return image;
}

image_rgba8 render_pattern(rasterizer & ras,
                           marker_rgba8 const& marker,
                           box2d<double> const & bbox,
                           agg::trans_affine const& tr)
{
    using pixfmt = agg::pixfmt_rgba32_pre;
    using renderer_base = agg::renderer_base<pixfmt>;
    using renderer = agg::renderer_scanline_aa_solid<renderer_base>;

    image_rgba8 image(bbox.width(), bbox.height());
    agg::rendering_buffer buf_out(image.bytes(), image.width(), image.height(), image.row_size());
    pixfmt pixf_out(buf_out);
    renderer_base rb(pixf_out);
    renderer r(rb);

    using const_rendering_buffer = util::rendering_buffer<image_rgba8>;
    using pixfmt_in = agg::pixfmt_alpha_blend_rgba<agg::blender_rgba32_pre, const_rendering_buffer, agg::pixel32_type>;

    image_rgba8 const& src = marker.get_data();
    const_rendering_buffer buf_in(src);
    pixfmt_in pixf(buf_in);

    agg::trans_affine final_tr(tr);
    final_tr.invert();

    using interpolator_type = agg::span_interpolator_linear<>;
    interpolator_type interpolator(final_tr);

    agg::span_allocator<agg::rgba8> sa;

    agg::image_filter_lut filter;
    filter.calculate(agg::image_filter_bilinear(), true);

    using img_accessor_type = agg::image_accessor_clone<pixfmt_in>;
    using span_gen_type = agg::span_image_resample_rgba_affine<img_accessor_type>;
    img_accessor_type ia(pixf);
    span_gen_type sg(ia, interpolator, filter);

    agg::scanline_u8 sl;
    ras.move_to_d(0, 0);
    ras.line_to_d(image.width(), 0);
    ras.line_to_d(image.width(), image.height());
    ras.line_to_d(0, image.height());

    agg::render_scanlines_aa(ras, sl, rb, sa, sg);

    return image;
}

} // namespace mapnik
