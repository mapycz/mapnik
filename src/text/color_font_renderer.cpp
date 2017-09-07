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
#include <mapnik/text/color_font_renderer.hpp>
#include <mapnik/font_engine_freetype.hpp>
#include <mapnik/image_scaling.hpp>
#include <mapnik/text/face.hpp>
#include <mapnik/agg_rasterizer.hpp>
#include <mapnik/image_util.hpp>

#pragma GCC diagnostic push
#include <mapnik/warning_ignore_agg.hpp>
#include "agg_rendering_buffer.h"
#include "agg_color_rgba.h"
#include "agg_scanline_u.h"
#include "agg_image_filters.h"
#include "agg_trans_bilinear.h"
#include "agg_span_allocator.h"
#include "agg_image_accessors.h"
#include "agg_span_image_filter_rgba.h"
#include "agg_renderer_base.h"
#include "agg_renderer_scanline.h"
#include "agg_ellipse.h"
#include "agg_pixfmt_gray.h"
#include "agg_scanline_bin.h"
#pragma GCC diagnostic pop

namespace mapnik
{

template<class PixFmt> class image_accessor_halo
{
public:
    using pixfmt_type = PixFmt;
    using color_type = typename pixfmt_type::color_type;
    using order_type = typename pixfmt_type::order_type;
    using value_type = typename pixfmt_type::value_type;
    using pixel_type = typename pixfmt_type::pixel_type;
    enum pix_width_e { pix_width = pixfmt_type::pix_width };

    image_accessor_halo(pixel_type halo_color, int radius, image_gray8 const& halo_img) :
        halo_color_(halo_color),
        nodata_color_(0),
        radius_(radius),
        halo_img_(halo_img)
    {
    }

private:
    const agg::int8u* pixel() const
    {
        int x = x_ + radius_, y = y_ + radius_;
        if (x < 0) x = 0;
        if (y < 0) y = 0;
        if (x >= halo_img_.width()) x = halo_img_.width() - 1;
        if (y >= halo_img_.height()) y = halo_img_.height() - 1;

        return halo_img_(x, y) ?
            reinterpret_cast<const agg::int8u*>(&halo_color_) :
            reinterpret_cast<const agg::int8u*>(&nodata_color_);
    }

public:
    const agg::int8u* span(int x, int y, unsigned len)
    {
        x_ = x0_ = x;
        y_ = y;
        return pixel();
    }

    const agg::int8u* next_x()
    {
        ++x_;
        return pixel();
    }

    const agg::int8u* next_y()
    {
        ++y_;
        x_ = x0_;
        return pixel();
    }

private:
    const pixel_type halo_color_;
    const pixel_type nodata_color_;
    const int radius_;
    image_gray8 halo_img_;
    int x_, x0_, y_;
};

template <typename Pixmap, typename ImageAccessor, typename ColorOrder>
void composite_color_glyph(Pixmap & pixmap,
                           ImageAccessor & img_accessor,
                           double width,
                           double height,
                           agg::trans_affine const& transform,
                           int x,
                           int y,
                           double angle,
                           box2d<double> const& bbox,
                           double opacity,
                           double halo_radius,
                           composite_mode_e comp_op)
{
    double scale = bbox.height() / height;
    double x0 = x;
    double y0 = y - bbox.maxy() / scale;
    double p[8];
    p[0] = x0;         p[1] = y0;
    p[2] = x0 + width; p[3] = y0;
    p[4] = x0 + width; p[5] = y0 + height;
    p[6] = x0;         p[7] = y0 + height;

    agg::trans_affine tr(transform);
    tr *= agg::trans_affine_translation(-x, -y);
    tr *= agg::trans_affine_rotation(angle);
    tr *= agg::trans_affine_scaling(scale);
    tr *= agg::trans_affine_translation(x, y);

    tr.transform(&p[0], &p[1]);
    tr.transform(&p[2], &p[3]);
    tr.transform(&p[4], &p[5]);
    tr.transform(&p[6], &p[7]);

    rasterizer ras;
    ras.move_to_d(p[0] - halo_radius, p[1] - halo_radius);
    ras.line_to_d(p[2] + halo_radius, p[3] - halo_radius);
    ras.line_to_d(p[4] + halo_radius, p[5] + halo_radius);
    ras.line_to_d(p[6] - halo_radius, p[7] + halo_radius);

    using color_type = agg::rgba8;
    using blender_type = agg::comp_op_adaptor_rgba_pre<color_type, ColorOrder>; // comp blender
    using pixfmt_comp_type = agg::pixfmt_custom_blend_rgba<blender_type, agg::rendering_buffer>;
    using renderer_base = agg::renderer_base<pixfmt_comp_type>;

    agg::scanline_u8 sl;
    agg::rendering_buffer buf(pixmap.bytes(),
                              pixmap.width(),
                              pixmap.height(),
                              pixmap.row_size());
    pixfmt_comp_type pixf(buf);
    pixf.comp_op(static_cast<agg::comp_op_e>(comp_op));
    renderer_base renb(pixf);

    agg::span_allocator<color_type> sa;
    agg::image_filter_bilinear filter_kernel;
    agg::image_filter_lut filter(filter_kernel, false);

    using interpolator_type = agg::span_interpolator_linear<agg::trans_affine>;
    using span_gen_type = agg::span_image_resample_rgba_affine<ImageAccessor>;
    using renderer_type = agg::renderer_scanline_aa_alpha<renderer_base,
                                                          agg::span_allocator<agg::rgba8>,
                                                          span_gen_type>;
    agg::trans_affine final_tr(p, 0, 0, width, height);
    final_tr.tx = std::floor(final_tr.tx + .5);
    final_tr.ty = std::floor(final_tr.ty + .5);
    interpolator_type interpolator(final_tr);
    span_gen_type sg(img_accessor, interpolator, filter);
    renderer_type rp(renb,sa, sg, static_cast<unsigned>(opacity * 255));
    agg::render_scanlines(ras, sl, rp);
}

template <typename T>
void composite_color_glyph(T & pixmap,
                           FT_Bitmap const& bitmap,
                           agg::trans_affine const& tr,
                           int x,
                           int y,
                           double angle,
                           box2d<double> const& bbox,
                           double opacity,
                           composite_mode_e comp_op)
{
    using pixfmt_type = agg::pixfmt_rgba32_pre;
    using img_accessor_type = agg::image_accessor_clone<pixfmt_type>;
    unsigned width = bitmap.width;
    unsigned height = bitmap.rows;
    agg::rendering_buffer glyph_buf(bitmap.buffer, width, height, width * pixfmt_type::pix_width);
    pixfmt_type glyph_pixf(glyph_buf);
    img_accessor_type img_accessor(glyph_pixf);

    using order_type = agg::order_bgra;
    composite_color_glyph<T, img_accessor_type, order_type>(
        pixmap, img_accessor, width, height, tr,
        x, y, angle, bbox, opacity, 0, comp_op);
}

template
void composite_color_glyph<image_rgba8>(image_rgba8 & pixmap,
                           FT_Bitmap const& bitmap,
                           agg::trans_affine const& tr,
                           int x,
                           int y,
                           double angle,
                           box2d<double> const& bbox,
                           double opacity,
                           composite_mode_e comp_op);

void draw_kernel(image_gray8 & kernel)
{
    agg::rasterizer_scanline_aa<> ras;
    agg::ellipse ellipse(kernel.width() / 2, kernel.height() / 2,
                         kernel.width() / 2, kernel.height() / 2);
    ras.add_path(ellipse);

    agg::rendering_buffer buf(kernel.data(), kernel.width(), kernel.height(), kernel.row_size());
    agg::pixfmt_gray8 pixfmt(buf);
    using renderer_base = agg::renderer_base<agg::pixfmt_gray8>;
    using renderer_bin = agg::renderer_scanline_bin_solid<renderer_base>;
    renderer_base rb(pixfmt);
    renderer_bin ren_bin(rb);
    ren_bin.color(agg::gray8(1));
    agg::scanline_bin sl_bin;
    agg::render_scanlines(ras, sl_bin, ren_bin);

}

halo_cache::kernel_type const& halo_cache::get_kernel(unsigned radius)
{
    auto ret = kernel_cache_.emplace(std::piecewise_construct,
                                     std::forward_as_tuple(radius),
                                     std::forward_as_tuple(2 * radius, 2 * radius));

    kernel_type & kernel = ret.first->second;

    if (!ret.second)
    {
        return kernel;
    }

    draw_kernel(kernel);
    return kernel;
}

void halo_cache::render_halo_img(pixfmt_type const& glyph_bitmap,
                                 img_type & halo_bitmap,
                                 int radius)
{
    image_gray8 const& kernel = get_kernel(radius);

    for (int x = 0; x < glyph_bitmap.width(); x++)
    {
        for (int y = 0; y < glyph_bitmap.height(); y++)
        {
            if (*reinterpret_cast<const pixfmt_type::pixel_type*>(glyph_bitmap.pix_ptr(x, y)))
            {
                for (int n = 0; n < kernel.width(); ++n)
                {
                    for (int m = 0; m < kernel.height(); ++m)
                    {
                        halo_bitmap(x + n, y + m) |= kernel(n, m);
                    }
                }
            }
        }
    }
}

halo_cache::img_type const& halo_cache::get(glyph_info const & glyph,
                                            pixfmt_type const& bitmap,
                                            double halo_radius)
{
    key_type key(glyph.face->family_name(), glyph.face->style_name(),
        glyph.glyph_index, glyph.height(), halo_radius);
    auto ret = cache_.emplace(std::piecewise_construct,
                              std::forward_as_tuple(key),
                              std::forward_as_tuple(bitmap.width() + 2 * halo_radius,
                                                    bitmap.height() + 2 * halo_radius));

    img_type & halo_img = ret.first->second;

    if (!ret.second)
    {
        return halo_img;
    }

    render_halo_img(bitmap, halo_img, halo_radius);
    return halo_img;
}

template <typename T>
void composite_color_glyph_halo(T & pixmap,
                                FT_Bitmap const& bitmap,
                                agg::trans_affine const& tr,
                                int x,
                                int y,
                                double angle,
                                box2d<double> const& bbox,
                                unsigned halo_color,
                                double opacity,
                                double halo_radius,
                                composite_mode_e comp_op,
                                glyph_info const& glyph,
                                halo_cache & cache)
{
    using pixfmt_type = agg::pixfmt_rgba32_pre;
    using img_accessor_type = image_accessor_halo<pixfmt_type>;
    unsigned width = bitmap.width;
    unsigned height = bitmap.rows;
    double scale = bbox.height() / height;
    agg::rendering_buffer glyph_buf(bitmap.buffer, width, height, width * pixfmt_type::pix_width);
    pixfmt_type glyph_pixf(glyph_buf);
    double scaled_radius = halo_radius / scale;
    image_gray8 const& halo_bitmap = cache.get(glyph, glyph_pixf, scaled_radius);
    img_accessor_type img_accessor(halo_color, scaled_radius, halo_bitmap);

    using order_type = agg::order_rgba;
    composite_color_glyph<T, img_accessor_type, order_type>(
        pixmap, img_accessor, width, height, tr, x, y,
        angle, bbox, opacity, halo_radius, comp_op);
}

template
void composite_color_glyph_halo<image_rgba8>(image_rgba8 & pixmap,
                                FT_Bitmap const& bitmap,
                                agg::trans_affine const& tr,
                                int x,
                                int y,
                                double angle,
                                box2d<double> const& bbox,
                                unsigned halo_color,
                                double opacity,
                                double halo_radius,
                                composite_mode_e comp_op,
                                glyph_info const& glyph,
                                halo_cache & cache);

} // namespace mapnik
