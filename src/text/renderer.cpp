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
#include <mapnik/text/renderer.hpp>
#include <mapnik/text/glyph_cache.hpp>
#include <mapnik/text/compositing.hpp>
#include <mapnik/text/text_properties.hpp>
#include <mapnik/font_engine_freetype.hpp>
#include <mapnik/image_compositing.hpp>
#include <mapnik/image_scaling.hpp>
#include <mapnik/text/face.hpp>
#include <mapnik/image_util.hpp>
#include <mapnik/image_any.hpp>
#include <mapnik/agg_rasterizer.hpp>
#include <mapnik/util/const_rendering_buffer.hpp>

#pragma GCC diagnostic push
#include <mapnik/warning_ignore_agg.hpp>
#include "agg_rendering_buffer.h"
#include "agg_pixfmt_rgba.h"
#include "agg_color_rgba.h"
#include "agg_scanline_u.h"
#include "agg_image_filters.h"
#include "agg_trans_bilinear.h"
#include "agg_span_allocator.h"
#include "agg_image_accessors.h"
#include "agg_span_image_filter_rgba.h"
#include "agg_renderer_base.h"
#include "agg_renderer_scanline.h"
#include "agg_pixfmt_gray.h"
#pragma GCC diagnostic pop

namespace mapnik
{

agg_text_renderer::agg_text_renderer(
    pixmap_type & pixmap,
    composite_mode_e comp_op,
    composite_mode_e halo_comp_op,
    double scale_factor)
    : comp_op_(comp_op),
      halo_comp_op_(halo_comp_op),
      scale_factor_(scale_factor),
      transform_(),
      halo_transform_(),
      pixmap_(pixmap),
      glyph_cache_(freetype_engine::get_glyph_cache())
{
}

void agg_text_renderer::set_transform(agg::trans_affine const& transform)
{
    transform_ = transform;
}

void agg_text_renderer::set_halo_transform(agg::trans_affine const& halo_transform)
{
    halo_transform_ = halo_transform;
}

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

template<class PixFmt> class image_accessor_colorize
{
public:
    using pixfmt_type = PixFmt;
    using color_type = typename pixfmt_type::color_type;
    using order_type = typename pixfmt_type::order_type;
    using value_type = typename pixfmt_type::value_type;
    using pixel_type = typename pixfmt_type::pixel_type;
    enum pix_width_e { pix_width = pixfmt_type::pix_width };

    image_accessor_colorize(pixel_type color, image_gray8 const& img) :
        color_(color), img_(img)
    {
    }

private:
    const agg::int8u* pixel() const
    {
        int x = x_, y = y_;
        if (x < 0) x = 0;
        if (y < 0) y = 0;
        if (x >= img_.width()) x = img_.width() - 1;
        if (y >= img_.height()) y = img_.height() - 1;
        color_ = (color_ & 0x00ffffff) | (static_cast<unsigned>(img_(x, y)) << 24);
        return reinterpret_cast<const agg::int8u*>(&color_);
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
    mutable pixel_type color_; // TODO: colorize after affine transforms
    image_gray8 const & img_;
    int x_, x0_, y_;
};


//=========================================span_image_resample_gray_affine
template<class Source, class Dest> 
class span_image_resample_gray_affine : 
public agg::span_image_resample_affine<Source>
{
public:
    typedef Source source_type;
    typedef typename source_type::color_type color_type;
    typedef agg::span_image_resample_affine<source_type> base_type;
    typedef typename base_type::interpolator_type interpolator_type;
    typedef typename color_type::value_type value_type;
    typedef typename color_type::long_type long_type;
    enum base_scale_e
    {
        base_shift      = color_type::base_shift,
        base_mask       = color_type::base_mask,
        downscale_shift = agg::image_filter_shift
    };

    using dest_type = Dest;
    using dest_color_type = typename dest_type::color_type;

    mapnik::color const & color_;

    //--------------------------------------------------------------------
    span_image_resample_gray_affine() {}
    span_image_resample_gray_affine(source_type& src, 
                                    interpolator_type& inter,
                                    const agg::image_filter_lut& filter,
                                    mapnik::color const & color) :
        base_type(src, inter, filter), color_(color) 
    {}


    //--------------------------------------------------------------------
    void generate(dest_color_type* span, int x, int y, unsigned len)
    {
        base_type::interpolator().begin(x + base_type::filter_dx_dbl(), 
                                        y + base_type::filter_dy_dbl(), len);

        long_type fg;

        int diameter     = base_type::filter().diameter();
        int filter_scale = diameter << agg::image_subpixel_shift;
        int radius_x     = (diameter * base_type::m_rx) >> 1;
        int radius_y     = (diameter * base_type::m_ry) >> 1;
        int len_x_lr     = 
            (diameter * base_type::m_rx + agg::image_subpixel_mask) >> 
                agg::image_subpixel_shift;

        const agg::int16* weight_array = base_type::filter().weight_array();

        do
        {
            base_type::interpolator().coordinates(&x, &y);

            x += base_type::filter_dx_int() - radius_x;
            y += base_type::filter_dy_int() - radius_y;

            fg = agg::image_filter_scale / 2;

            int y_lr = y >> agg::image_subpixel_shift;
            int y_hr = ((agg::image_subpixel_mask - (y & agg::image_subpixel_mask)) * 
                            base_type::m_ry_inv) >> 
                                agg::image_subpixel_shift;
            int total_weight = 0;
            int x_lr = x >> agg::image_subpixel_shift;
            int x_hr = ((agg::image_subpixel_mask - (x & agg::image_subpixel_mask)) * 
                            base_type::m_rx_inv) >> 
                                agg::image_subpixel_shift;

            int x_hr2 = x_hr;
            const value_type* fg_ptr = 
                (const value_type*)base_type::source().span(x_lr, y_lr, len_x_lr);
            for(;;)
            {
                int weight_y = weight_array[y_hr];
                x_hr = x_hr2;
                for(;;)
                {
                    int weight = (weight_y * weight_array[x_hr] + 
                                 agg::image_filter_scale / 2) >> 
                                 downscale_shift;

                    fg += *fg_ptr * weight;
                    total_weight += weight;
                    x_hr  += base_type::m_rx_inv;
                    if(x_hr >= filter_scale) break;
                    fg_ptr = (const value_type*)base_type::source().next_x();
                }
                y_hr += base_type::m_ry_inv;
                if(y_hr >= filter_scale) break;
                fg_ptr = (const value_type*)base_type::source().next_y();
            }

            fg /= total_weight;
            if(fg < 0) fg = 0;
            if(fg > base_mask) fg = base_mask;

            //span->v = (value_type)fg;
            //span->a = base_mask;
            span->r = (value_type)color_.red_;
            span->g = (value_type)color_.green_;
            span->b = (value_type)color_.blue_;
            span->a = (value_type)fg;

            ++span;
            ++base_type::interpolator();
        } while(--len);
    }
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
                           composite_mode_e comp_op,
                           mapnik::color const& color)
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
    tr *= agg::trans_affine_translation(-halo_radius, 0);
    tr *= agg::trans_affine_rotation(angle);
    tr *= agg::trans_affine_scaling(scale);
    tr *= agg::trans_affine_translation(x, y);

    tr.transform(&p[0], &p[1]);
    tr.transform(&p[2], &p[3]);
    tr.transform(&p[4], &p[5]);
    tr.transform(&p[6], &p[7]);

    rasterizer ras;
    ras.move_to_d(p[0], p[1]);
    ras.line_to_d(p[2], p[3]);
    ras.line_to_d(p[4], p[5]);
    ras.line_to_d(p[6], p[7]);

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
    using span_gen_type = span_image_resample_gray_affine<ImageAccessor, pixfmt_comp_type>;
    using renderer_type = agg::renderer_scanline_aa_alpha<renderer_base,
                                                          agg::span_allocator<agg::rgba8>,
                                                          span_gen_type>;
    agg::trans_affine final_tr(p, 0, 0, width, height);
    final_tr.tx = std::floor(final_tr.tx + .5);
    final_tr.ty = std::floor(final_tr.ty + .5);
    interpolator_type interpolator(final_tr);
    span_gen_type sg(img_accessor, interpolator, filter, color);
    renderer_type rp(renb,sa, sg, static_cast<unsigned>(opacity * 255));
    agg::render_scanlines(ras, sl, rp);
}

void composite_glyph(image_rgba8 & dst,
                     image_gray8 const& src,
                     unsigned rgba,
                     agg::trans_affine const& tr,
                     int x,
                     int y,
                     double angle,
                     box2d<double> const& bbox,
                     double opacity,
                     double halo_radius,
                     composite_mode_e comp_op)
{
    //using pixfmt_type = agg::pixfmt_gray8_pre;
    //using img_accessor_type = image_accessor_colorize<pixfmt_type>;
    //img_accessor_type img_accessor(rgba, src);
    using const_rendering_buffer = util::rendering_buffer<image_gray8>;
    using pixfmt_type = agg::pixfmt_alpha_blend_gray<
        agg::blender_gray8_pre, const_rendering_buffer>;
    using img_accessor_type = agg::image_accessor_clone<pixfmt_type>;
    const_rendering_buffer glyph_buf(src);
    pixfmt_type glyph_pixf(glyph_buf);
    img_accessor_type img_accessor(glyph_pixf);
    const mapnik::color c(rgba);


    using order_type = agg::order_rgba;
    composite_color_glyph<image_rgba8, img_accessor_type, order_type>(
        dst, img_accessor, src.width(), src.height(), tr,
        x, y, angle, bbox, opacity, halo_radius, comp_op, c);
}

void agg_text_renderer::render(glyph_positions const& pos)
{
    const bool is_mono = (pos.size() != 0) && pos.begin()->glyph.format.text_mode == TEXT_MODE_MONO;
    std::vector<glyph_t> glyphs;

    for (auto const& glyph_pos : pos)
    {
        glyph_info const& glyph = glyph_pos.glyph;
        double size = glyph.format.text_size * scale_factor_;
        pixel_position pos = glyph_pos.pos + glyph.offset.rotate(glyph_pos.rot);
        if (is_mono)
        {
            pos.x = std::round(pos.x);
            pos.y = std::round(pos.y);
        }
        box2d<double> bbox(0, glyph_pos.glyph.ymin, 1, glyph_pos.glyph.ymax);
        glyphs.emplace_back(glyph, pos, glyph_pos.rot, size, bbox);
    }

    const int height = pixmap_.height();
    pixel_position base_point = pos.get_base_point();
    if (is_mono)
    {
        base_point.x = std::round(base_point.x);
        base_point.y = std::round(base_point.y);
    }
    const pixel_position start(
        base_point.x + transform_.tx,
        base_point.y - transform_.ty);
    const pixel_position start_halo(
        base_point.x + halo_transform_.tx,
        base_point.y - halo_transform_.ty);

    // halo
    for (auto const& glyph : glyphs)
    {
        const glyph_cache::value_type * glyph_val = glyph_cache_.get(
            glyph.info, glyph.info.format.halo_radius);
        if (glyph_val)
        {
            const double halo_radius = glyph.info.format.halo_radius * scale_factor_;
            const int x = start_halo.x + glyph.pos.x;
            const int y = start_halo.y - glyph.pos.y;
            box2d<double> halo_bbox(glyph.bbox);
            halo_bbox.pad(halo_radius);
            composite_glyph(pixmap_,
                            glyph_val->img,
                            glyph.info.format.halo_fill.rgba(),
                            halo_transform_,
                            x, y,
                            -glyph.rot.angle(),
                            halo_bbox,
                            glyph.info.format.halo_opacity,
                            glyph.info.format.halo_radius * glyph_cache_.scale,
                            halo_comp_op_);
        }
    }

    // render actual text
    for (auto & glyph : glyphs)
    {
        const glyph_cache::value_type * glyph_val = glyph_cache_.get(glyph.info, 0.0);
        if (glyph_val)
        {
            const int x = start.x + glyph.pos.x;
            const int y = start.y - glyph.pos.y;
            composite_glyph(pixmap_,
                            glyph_val->img,
                            glyph.info.format.fill.rgba(),
                            transform_,
                            x, y,
                            -glyph.rot.angle(),
                            glyph.bbox,
                            glyph.info.format.text_opacity,
                            0.0,
                            comp_op_);
        }
    }
}

} // namespace mapnik
