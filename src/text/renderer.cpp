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
#pragma GCC diagnostic pop

namespace mapnik
{

text_renderer::text_renderer (halo_rasterizer_e rasterizer, composite_mode_e comp_op,
                              composite_mode_e halo_comp_op, double scale_factor, stroker_ptr stroker)
    : rasterizer_(rasterizer),
      comp_op_(comp_op),
      halo_comp_op_(halo_comp_op),
      scale_factor_(scale_factor),
      glyphs_(),
      stroker_(stroker),
      transform_(),
      halo_transform_()
{}

void text_renderer::set_transform(agg::trans_affine const& transform)
{
    transform_ = transform;
}

void text_renderer::set_halo_transform(agg::trans_affine const& halo_transform)
{
    halo_transform_ = halo_transform;
}

FT_Error text_renderer::select_closest_size(glyph_info const& glyph, FT_Face & face) const
{
    int scaled_size = static_cast<int>(glyph.format.text_size * scale_factor_);
    int best_match = 0;
    int diff = std::abs(scaled_size - face->available_sizes[0].width);
    for (int i = 1; i < face->num_fixed_sizes; ++i)
    {
        int ndiff = std::abs(scaled_size - face->available_sizes[i].height);
        if (ndiff < diff)
        {
            best_match = i;
            diff = ndiff;
        }
    }
    return FT_Select_Size(face, best_match);
}

void text_renderer::prepare_glyphs(glyph_positions const& positions, bool is_mono)
{
    FT_Matrix matrix;
    FT_Vector pen;
    FT_Error  error;

    glyphs_.clear();
    glyphs_.reserve(positions.size());

    for (auto const& glyph_pos : positions)
    {
        glyph_info const& glyph = glyph_pos.glyph;
        FT_Int32 load_flags = FT_LOAD_DEFAULT;

        if (glyph.format.text_mode == TEXT_MODE_DEFAULT)
        {
            load_flags |= FT_LOAD_NO_HINTING;
        }

        FT_Face face = glyph.face->get_face();
        if (glyph.face->is_color())
        {
            load_flags |= FT_LOAD_COLOR;
        }

        if (face->num_fixed_sizes > 0)
        {
            error = select_closest_size(glyph, face);
            if (error) continue;
        }
        else
        {
            glyph.face->set_character_sizes(glyph.format.text_size * scale_factor_);
        }

        double size = glyph.format.text_size * scale_factor_;
        matrix.xx = static_cast<FT_Fixed>( glyph_pos.rot.cos * 0x10000L);
        matrix.xy = static_cast<FT_Fixed>(-glyph_pos.rot.sin * 0x10000L);
        matrix.yx = static_cast<FT_Fixed>( glyph_pos.rot.sin * 0x10000L);
        matrix.yy = static_cast<FT_Fixed>( glyph_pos.rot.cos * 0x10000L);

        pixel_position pos = glyph_pos.pos + glyph.offset.rotate(glyph_pos.rot);
        if (is_mono)
        {
            pos.x = std::round(pos.x);
            pos.y = std::round(pos.y);
        }
        pen.x = static_cast<FT_Pos>(pos.x * 64);
        pen.y = static_cast<FT_Pos>(pos.y * 64);

        FT_Set_Transform(face, &matrix, &pen);
        error = FT_Load_Glyph(face, glyph.glyph_index, load_flags);
        if (error) continue;
        FT_Glyph image;
        error = FT_Get_Glyph(face->glyph, &image);
        if (error) continue;
        box2d<double> bbox(0, glyph_pos.glyph.ymin, 1, glyph_pos.glyph.ymax);
        glyphs_.emplace_back(glyph, image, pos, glyph_pos.rot, size, bbox);
    }
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

void composite_glyph(image_rgba8 & dst,
                     image_gray8 const& src,
                     unsigned rgba,
                     agg::trans_affine const& tr,
                     int x,
                     int y,
                     double angle,
                     box2d<double> const& bbox,
                     double opacity,
                     composite_mode_e comp_op)
{
    //using const_rendering_buffer = util::rendering_buffer<image_rgba8>;
    //using pixfmt_type = agg::pixfmt_alpha_blend_rgba<
        //agg::blender_rgba32_pre, const_rendering_buffer, agg::pixel32_type>;
    using pixfmt_type = agg::pixfmt_rgba32_pre;
    using img_accessor_type = image_accessor_colorize<pixfmt_type>;
    img_accessor_type img_accessor(rgba, src);

    using order_type = agg::order_rgba;
    composite_color_glyph<image_rgba8, img_accessor_type, order_type>(
        dst, img_accessor, src.width(), src.height(), tr,
        x, y, angle, bbox, opacity, 0, comp_op);
}

void halo_cache::render_halo_img(pixfmt_type const& glyph_bitmap,
                                 img_type & halo_bitmap,
                                 int radius)
{
    for (int x = 0; x < glyph_bitmap.width(); x++)
    {
        for (int y = 0; y < glyph_bitmap.height(); y++)
        {
            if (*reinterpret_cast<const pixfmt_type::pixel_type*>(glyph_bitmap.pix_ptr(x, y)))
            {
                for (int n = -radius; n <= radius; ++n)
                {
                    for (int m = -radius; m <= radius; ++m)
                    {
                        halo_bitmap(radius + x + n, radius + y + m) = 1;
                    }
                }
            }
        }
    }
}

image_gray8 const& halo_cache::get(glyph_info const & glyph,
                                   pixfmt_type const& bitmap,
                                   double halo_radius)
{
    key_type key(glyph.face->family_name(), glyph.face->style_name(),
        glyph.glyph_index, glyph.height, halo_radius);
    value_type & halo_img_ptr = cache_[key];

    if (halo_img_ptr)
    {
        return *halo_img_ptr;
    }

    halo_img_ptr.reset(new img_type(bitmap.width() + 2 * halo_radius,
                                    bitmap.height() + 2 * halo_radius));
    img_type & halo_img = *halo_img_ptr;
    render_halo_img(bitmap, halo_img, halo_radius);
    return halo_img;
}

/*
void glyph_cache::render_halo_img(
    FT_Bitmap const& glyph_bitmap,
    img_type & halo_bitmap,
    int radius)
{
    using pixfmt_type = agg::pixfmt_rgba32_pre;
    using img_accessor_type = image_accessor_halo<pixfmt_type>;
    const unsigned width = bitmap.width;
    const unsigned height = bitmap.rows;
    const double scale = bbox.height() / height;
    agg::rendering_buffer glyph_buf(bitmap.buffer, width, height, width * pixfmt_type::pix_width);
    pixfmt_type glyph_pixf(glyph_buf);
    const double scaled_radius = halo_radius / scale;
    img_accessor_type img_accessor(halo_color, scaled_radius, halo_bitmap);

    for (int x = 0; x < glyph_bitmap.width(); x++)
    {
        for (int y = 0; y < glyph_bitmap.height(); y++)
        {
            if (*reinterpret_cast<const pixfmt_type::pixel_type*>(glyph_bitmap.pix_ptr(x, y)))
            {
                for (int n = -radius; n <= radius; ++n)
                {
                    for (int m = -radius; m <= radius; ++m)
                    {
                        halo_bitmap(radius + x + n, radius + y + m) = 1;
                    }
                }
            }
        }
    }
}
*/

void glyph_cache::render_halo(
    image_gray8 & dst,
    FT_Bitmap const & src,
    int radius) const
{
    unsigned char * buff = src.buffer;

    for (int j = 0; j < src.rows; ++j)
    {
        unsigned char * row = buff;
        unsigned b = 0;
        for (int i = 0, p = 0; i < src.width; ++i, ++p)
        {
            if (p % 8 == 0)
            {
                b = *row++;
            }
            if (b & 0x80)
            {
                for (int n = -radius; n <= radius; ++n)
                {
                    for (int m = -radius; m <= radius; ++m)
                    {
                        dst(radius + i + n, radius + j + m) = 1;
                    }
                }
            }
            b <<= 1;
        }
        buff += src.pitch;
    }
}

const glyph_cache::value_type * glyph_cache::get(glyph_info const & glyph)
{
    glyph_cache_key key{*glyph.face, glyph.glyph_index, glyph.height};

    if (auto it = cache_.find(key); it != cache_.end())
    {
        return &it->second;
    }

    return render(key, glyph);
}

const glyph_cache::value_type * glyph_cache::get_halo(glyph_info const & glyph, double halo_radius)
{
    glyph_halo_cache_key key{*glyph.face, glyph.glyph_index, glyph.height, halo_radius};

    if (auto it = halo_cache_.find(key); it != halo_cache_.end())
    {
        return &it->second;
    }

    return render_halo(key, glyph, halo_radius * 2.0);
}

struct done_glyph
{
     FT_Glyph & glyph;

     ~done_glyph()
     {
        FT_Done_Glyph(glyph);
     }
};

FT_Error glyph_cache::select_closest_size(glyph_info const& glyph, FT_Face & face) const
{
    int scaled_size = static_cast<int>(glyph.format.text_size * 2.0/*scale_factor_*/);
    int best_match = 0;
    int diff = std::abs(scaled_size - face->available_sizes[0].width);
    for (int i = 1; i < face->num_fixed_sizes; ++i)
    {
        int ndiff = std::abs(scaled_size - face->available_sizes[i].height);
        if (ndiff < diff)
        {
            best_match = i;
            diff = ndiff;
        }
    }
    return FT_Select_Size(face, best_match);
}

const glyph_cache::value_type * glyph_cache::render(glyph_cache_key const & key, glyph_info const & glyph)
{
    FT_Int32 load_flags = FT_LOAD_DEFAULT;

    if (glyph.format.text_mode == TEXT_MODE_DEFAULT)
    {
        load_flags |= FT_LOAD_NO_HINTING;
    }

    FT_Face face = glyph.face->get_face();
    if (glyph.face->is_color())
    {
        load_flags |= FT_LOAD_COLOR;
    }

    if (face->num_fixed_sizes > 0)
    {
        if (select_closest_size(glyph, face))
        {
            return nullptr;
        }
    }
    else
    {
        glyph.face->set_character_sizes(glyph.format.text_size * 2.0 /*scale_factor_*/);
    }

    FT_Set_Transform(face, nullptr, nullptr);

    if (FT_Load_Glyph(face, glyph.glyph_index, load_flags))
    {
        return nullptr;
    }

    FT_Glyph image;
    if (FT_Get_Glyph(face->glyph, &image))
    {
        return nullptr;
    }

    if (image->format == FT_GLYPH_FORMAT_BITMAP)
    {
        FT_BitmapGlyph bit = reinterpret_cast<FT_BitmapGlyph>(image);
        auto result = cache_.emplace(
            std::piecewise_construct,
            std::forward_as_tuple(key),
            std::forward_as_tuple(
                bit->bitmap.width, bit->bitmap.rows,
                bit->left, bit->bitmap.rows /* TODO: do we have better height val here? */ - bit->top));
        if (!result.second) { return nullptr; }
        value_type & val = result.first->second;
        img_type & glyph_img = val.img;
        switch (bit->bitmap.pixel_mode)
        {
            case FT_PIXEL_MODE_BGRA:
                /* TODO: color fonts
                composite_color_glyph(glyph_img,
                                      bit->bitmap,
                                      agg::trans_affine(),
                                      0, 0,
                                      0,
                                      box2d<double>(0, 0, bit->bitmap.width, bit->bitmap.rows),
                                      1.0,
                                      src_over);
                return &glyph_img;
                */
                return nullptr;
            case FT_PIXEL_MODE_MONO:
                composite_bitmap_mono(glyph_img, bit->bitmap, 0, 0);
                return &val;
        }
    }
    else
    {
        FT_Render_Mode mode = (glyph.format.text_mode == TEXT_MODE_DEFAULT)
            ? FT_RENDER_MODE_NORMAL : FT_RENDER_MODE_MONO;
        if (!FT_Glyph_To_Bitmap(&image, mode, 0, 1))
        {
            done_glyph release_glyph{image};
            FT_BitmapGlyph bit = reinterpret_cast<FT_BitmapGlyph>(image);
            auto result = cache_.emplace(
                std::piecewise_construct,
                std::forward_as_tuple(key),
                std::forward_as_tuple(bit->bitmap.width, bit->bitmap.rows,
                    bit->left, bit->bitmap.rows /* TODO: do we have better height val here? */ - bit->top));
            if (!result.second) { return nullptr; }
            value_type & val = result.first->second;
            img_type & glyph_img = val.img;
            switch (bit->bitmap.pixel_mode)
            {
                case FT_PIXEL_MODE_GRAY:
                    composite_bitmap(glyph_img, bit->bitmap, 0, 0);
                    break;
                case FT_PIXEL_MODE_MONO:
                    composite_bitmap_mono(glyph_img, bit->bitmap, 0, 0);
                    break;
            }
            return &val;
        }
    }

    return nullptr;
}

const glyph_cache::value_type * glyph_cache::render_halo(
    glyph_halo_cache_key const & key,
    glyph_info const & glyph,
    double halo_radius)
{
    FT_Int32 load_flags = FT_LOAD_DEFAULT;

    if (glyph.format.text_mode == TEXT_MODE_DEFAULT)
    {
        load_flags |= FT_LOAD_NO_HINTING;
    }

    FT_Face face = glyph.face->get_face();
    if (glyph.face->is_color())
    {
        load_flags |= FT_LOAD_COLOR;
    }

    if (face->num_fixed_sizes > 0)
    {
        if (select_closest_size(glyph, face))
        {
            return nullptr;
        }
    }
    else
    {
        glyph.face->set_character_sizes(glyph.format.text_size * 2.0 /*scale_factor_*/);
    }

    FT_Set_Transform(face, nullptr, nullptr);

    if (FT_Load_Glyph(face, glyph.glyph_index, load_flags))
    {
        return nullptr;
    }

    FT_Glyph image;
    // TODO: release the FT_Glyph?
    if (FT_Get_Glyph(face->glyph, &image))
    {
        return nullptr;
    }

    if (image->format == FT_GLYPH_FORMAT_BITMAP)
    {
        FT_BitmapGlyph bit = reinterpret_cast<FT_BitmapGlyph>(image);
        const int w = bit->bitmap.width + 2 * halo_radius;
        const int h = bit->bitmap.rows + 2 * halo_radius;
        auto result = halo_cache_.emplace(
            std::piecewise_construct,
            std::forward_as_tuple(key),
            std::forward_as_tuple(
                bit->bitmap.width + 2 * halo_radius,
                bit->bitmap.rows + 2 * halo_radius,
                bit->left, h /* TODO: do we have better height val here? */ - bit->top));
        if (!result.second) { return nullptr; }
        value_type & val = result.first->second;
        img_type & glyph_img = val.img;
        switch (bit->bitmap.pixel_mode)
        {
            case FT_PIXEL_MODE_BGRA:
                /*
                composite_color_glyph(glyph_img,
                                      bit->bitmap,
                                      agg::trans_affine(),
                                      0, 0,
                                      0,
                                      box2d<double>(0, 0, bit->bitmap.width, bit->bitmap.rows),
                                      1.0,
                                      src_over);
                return &glyph_img;
                */
                // TODO: support color fonts
                return nullptr;
            case FT_PIXEL_MODE_MONO:
                render_halo(glyph_img, bit->bitmap, halo_radius);
                return &val;
        }
    }
    else
    {
        FT_Glyph g;
        if (FT_Glyph_Copy(image, &g)) { return nullptr; }
        done_glyph release_glyph{g};
        FT_Glyph_Transform(g, nullptr, nullptr);
        stroker & strk = *font_manager_.get_stroker();
        strk.init(halo_radius);
        FT_Glyph_Stroke(&g, strk.get(), 1);
        if (!FT_Glyph_To_Bitmap(&g, FT_RENDER_MODE_NORMAL, 0, 1))
        {
            FT_BitmapGlyph bit = reinterpret_cast<FT_BitmapGlyph>(g);
            auto result = halo_cache_.emplace(
                std::piecewise_construct,
                std::forward_as_tuple(key),
                std::forward_as_tuple(
                    bit->bitmap.width, bit->bitmap.rows,
                    bit->left, bit->bitmap.rows /* TODO: do we have better height val here? */ - bit->top));

            /*
            if (bit->bitmap.width && bit->bitmap.rows)
            {
            std::clog << bit->bitmap.width << " ; " << bit->bitmap.rows << " : " << bit->top << " : " << bit->left << std::endl;
            image_rgba8 i(bit->bitmap.width, bit->bitmap.rows);
            rasterizer ras;
            composite_bitmap(i, &bit->bitmap,
                        color(0, 0, 0).rgba(),
                        0, 0, 1.0, src_over, ras);
            save_to_file(i, std::to_string(glyph.glyph_index) + ".png", "png32");
            }
            */



            if (!result.second) { return nullptr; }
            value_type & val = result.first->second;
            img_type & glyph_img = val.img;
            composite_bitmap(glyph_img, bit->bitmap, 0, 0);
            return &val;
        }
    }

    return nullptr;
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

template <typename T>
agg_text_renderer<T>::agg_text_renderer (pixmap_type & pixmap,
                                         halo_rasterizer_e halo_rasterizer,
                                         rasterizer const & ras,
                                         composite_mode_e comp_op,
                                         composite_mode_e halo_comp_op,
                                         double scale_factor,
                                         stroker_ptr stroker)
    : text_renderer(halo_rasterizer, comp_op, halo_comp_op,
                    scale_factor, stroker),
      pixmap_(pixmap),
      ras_(ras)
{}

template <typename T>
void agg_text_renderer<T>::render(glyph_positions const& pos)
{
    const bool is_mono = (pos.size() != 0) && pos.begin()->glyph.format.text_mode == TEXT_MODE_MONO;
    prepare_glyphs(pos, is_mono);
    const int height = pixmap_.height();
    pixel_position base_point = pos.get_base_point();
    if (is_mono)
    {
        base_point.x = std::round(base_point.x);
        base_point.y = std::round(base_point.y);
    }
    const pixel_position start(
        base_point.x + transform_.tx,
        height - base_point.y + transform_.ty);
    const pixel_position start_halo(
        base_point.x + halo_transform_.tx,
        height - base_point.y + halo_transform_.ty);

    for (auto const& glyph : glyphs_)
    {
        const glyph_cache::value_type * glyph_val = glyph_cache_.get_halo(glyph.info, glyph.info.format.halo_radius);
        if (glyph_val)
        {
            const double halo_radius = glyph.info.format.halo_radius * scale_factor_;
            int x = start_halo.x + glyph.pos.x - halo_radius;
            int y = height - start_halo.y - glyph.pos.y;
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
                            halo_comp_op_);
        }
    }

    // render actual text
    for (auto & glyph : glyphs_)
    {
        const glyph_cache::value_type * glyph_val = glyph_cache_.get(glyph.info);
        if (glyph_val)
        {
            int x = start.x + glyph.pos.x;
            int y = height - start.y - glyph.pos.y;
            composite_glyph(pixmap_,
                            glyph_val->img,
                            glyph.info.format.fill.rgba(),
                            transform_,
                            x, y,
                            -glyph.rot.angle(),
                            glyph.bbox,
                            glyph.info.format.text_opacity,
                            comp_op_);
        }
    }
}

template <typename T>
void agg_text_renderer<T>::render_halo(unsigned char *buffer,
                                       unsigned width,
                                       unsigned height,
                                       unsigned pixel_width,
                                       unsigned rgba,
                                       int x1,
                                       int y1,
                                       double halo_radius,
                                       double opacity,
                                       composite_mode_e comp_op)
{
    int x, y;
    if (halo_radius < 1.0)
    {
        for (x=0; x < width; x++)
        {
            for (y=0; y < height; y++)
            {
                int gray = buffer[(y * width + x) * pixel_width + pixel_width - 1];
                if (gray)
                {
                    mapnik::composite_pixel(pixmap_, comp_op, x+x1-1, y+y1-1, rgba, gray*halo_radius*halo_radius, opacity);
                    mapnik::composite_pixel(pixmap_, comp_op, x+x1,   y+y1-1, rgba, gray*halo_radius, opacity);
                    mapnik::composite_pixel(pixmap_, comp_op, x+x1+1, y+y1-1, rgba, gray*halo_radius*halo_radius, opacity);

                    mapnik::composite_pixel(pixmap_, comp_op, x+x1-1, y+y1,   rgba, gray*halo_radius, opacity);
                    mapnik::composite_pixel(pixmap_, comp_op, x+x1,   y+y1,   rgba, gray, opacity);
                    mapnik::composite_pixel(pixmap_, comp_op, x+x1+1, y+y1,   rgba, gray*halo_radius, opacity);

                    mapnik::composite_pixel(pixmap_, comp_op, x+x1-1, y+y1+1, rgba, gray*halo_radius*halo_radius, opacity);
                    mapnik::composite_pixel(pixmap_, comp_op, x+x1,   y+y1+1, rgba, gray*halo_radius, opacity);
                    mapnik::composite_pixel(pixmap_, comp_op, x+x1+1, y+y1+1, rgba, gray*halo_radius*halo_radius, opacity);
                }
            }
        }
    }
    else
    {
        for (x=0; x < width; x++)
        {
            for (y=0; y < height; y++)
            {
                int gray = buffer[(y * width + x) * pixel_width + pixel_width - 1];
                if (gray)
                {
                    for (int n=-halo_radius; n <=halo_radius; ++n)
                        for (int m=-halo_radius; m <= halo_radius; ++m)
                            mapnik::composite_pixel(pixmap_, comp_op, x+x1+m, y+y1+n, rgba, gray, opacity);
                }
            }
        }
    }
}

template class agg_text_renderer<image_rgba8>;

} // namespace mapnik
