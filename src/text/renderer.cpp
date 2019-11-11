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

const glyph_cache::img_type * glyph_cache::get(glyph_info const & glyph)
{
    glyph_cache_key key{*glyph.face, glyph.glyph_index, glyph.height};

    if (auto it = cache_.find(key); it != cache_.end())
    {
        return &it->second;
    }

    return render(glyph);
}

const glyph_cache::img_type * halo_cache::render(glyph_info const& glyph)
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
        if (error = select_closest_size(glyph, face))
        {
            return nullptr;
        }
    }
    else
    {
        glyph.face->set_character_sizes(glyph.format.text_size * scale_factor_);
    }

    if (error = FT_Load_Glyph(face, glyph.glyph_index, load_flags))
    {
        return nullptr;
    }

    FT_Glyph image;
    if (error = FT_Get_Glyph(face->glyph, &image))
    {
        return nullptr;
    }




    box2d<double> bbox(0, glyph_pos.glyph.ymin, 1, glyph_pos.glyph.ymax);
    glyph_img_ptr.reset(new img_type(bitmap.width() + 2 * halo_radius,
                                     bitmap.height() + 2 * halo_radius));
    img_type & glyph_img = *glyph_img_ptr;
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
    bool is_mono = (pos.size() != 0) && pos.begin()->glyph.format.text_mode == TEXT_MODE_MONO;
    prepare_glyphs(pos, is_mono);
    FT_Error  error;
    FT_Vector start;
    FT_Vector start_halo;
    int height = pixmap_.height();
    pixel_position base_point = pos.get_base_point();
    if (is_mono)
    {
        base_point.x = std::round(base_point.x);
        base_point.y = std::round(base_point.y);
    }
    start.x = static_cast<FT_Pos>(base_point.x * (1 << 6));
    start.y = static_cast<FT_Pos>((height - base_point.y) * (1 << 6));
    start_halo = start;
    start.x += transform_.tx * 64;
    start.y += transform_.ty * 64;
    start_halo.x += halo_transform_.tx * 64;
    start_halo.y += halo_transform_.ty * 64;

    FT_Matrix halo_matrix;
    halo_matrix.xx = halo_transform_.sx  * 0x10000L;
    halo_matrix.xy = halo_transform_.shx * 0x10000L;
    halo_matrix.yy = halo_transform_.sy  * 0x10000L;
    halo_matrix.yx = halo_transform_.shy * 0x10000L;

    FT_Matrix matrix;
    matrix.xx = transform_.sx  * 0x10000L;
    matrix.xy = transform_.shx * 0x10000L;
    matrix.yy = transform_.sy  * 0x10000L;
    matrix.yx = transform_.shy * 0x10000L;

    // default formatting
    double halo_radius = 0;
    color black(0,0,0);
    unsigned fill = black.rgba();
    unsigned halo_fill = black.rgba();
    double text_opacity = 1.0;
    double halo_opacity = 1.0;

    for (auto const& glyph : glyphs_)
    {
        halo_fill = glyph.info.format.halo_fill.rgba();
        halo_opacity = glyph.info.format.halo_opacity;
        halo_radius = glyph.info.format.halo_radius * scale_factor_;
        // make sure we've got reasonable values.
        if (halo_radius <= 0.0 || halo_radius > 1024.0) continue;
        FT_Glyph g;
        error = FT_Glyph_Copy(glyph.image, &g);
        if (!error)
        {
            FT_Glyph_Transform(g, &halo_matrix, &start_halo);
            if (rasterizer_ == HALO_RASTERIZER_FULL)
            {
                if (g->format != FT_GLYPH_FORMAT_OUTLINE)
                {
                    MAPNIK_LOG_WARN(agg_text_renderer) << "HALO_RASTERIZER_FULL only works with vectorial glyphs.";
                    continue;
                }

                stroker_->init(halo_radius);
                FT_Glyph_Stroke(&g, stroker_->get(), 1);
                error = FT_Glyph_To_Bitmap(&g, FT_RENDER_MODE_NORMAL, 0, 1);
                if (!error)
                {
                    FT_BitmapGlyph bit = reinterpret_cast<FT_BitmapGlyph>(g);
                    if (bit->bitmap.pixel_mode != FT_PIXEL_MODE_BGRA)
                    {
                        composite_bitmap(pixmap_,
                                         &bit->bitmap,
                                         halo_fill,
                                         bit->left,
                                         height - bit->top,
                                         halo_opacity,
                                         halo_comp_op_,
                                         ras_);
                    }
                }
            }
            else
            {
                if (g->format == FT_GLYPH_FORMAT_OUTLINE)
                {
                    error = FT_Glyph_To_Bitmap(&g, FT_RENDER_MODE_NORMAL, 0, 1);
                    if (error)
                    {
                        continue;
                    }
                }
                FT_BitmapGlyph bit = reinterpret_cast<FT_BitmapGlyph>(g);
                if (bit->bitmap.pixel_mode == FT_PIXEL_MODE_BGRA)
                {
                    int x = (start.x >> 6) + glyph.pos.x;
                    int y = height - (start.y >> 6) - glyph.pos.y;
                    composite_color_glyph_halo(pixmap_,
                                               bit->bitmap,
                                               transform_,
                                               x, y,
                                               -glyph.rot.angle(),
                                               glyph.bbox,
                                               halo_fill,
                                               halo_opacity,
                                               halo_radius,
                                               comp_op_,
                                               glyph.info,
                                               halo_cache_);
                }
                else
                {
                    render_halo(bit->bitmap.buffer,
                                bit->bitmap.width,
                                bit->bitmap.rows,
                                1,
                                halo_fill,
                                bit->left,
                                height - bit->top,
                                halo_radius,
                                halo_opacity,
                                halo_comp_op_);
                }
            }
        }
        FT_Done_Glyph(g);
    }

    // render actual text
    for (auto & glyph : glyphs_)
    {
        fill = glyph.info.format.fill.rgba();
        text_opacity = glyph.info.format.text_opacity;

        FT_Glyph_Transform(glyph.image, &matrix, &start);
        error = 0;
        if (glyph.image->format == FT_GLYPH_FORMAT_BITMAP)
        {
            FT_BitmapGlyph bit = reinterpret_cast<FT_BitmapGlyph>(glyph.image);
            int x = (start.x >> 6) + glyph.pos.x;
            int y = height - (start.y >> 6) - glyph.pos.y;
            switch (bit->bitmap.pixel_mode)
            {
                case FT_PIXEL_MODE_BGRA:
                    composite_color_glyph(pixmap_,
                                          bit->bitmap,
                                          transform_,
                                          x, y,
                                          -glyph.rot.angle(),
                                          glyph.bbox,
                                          text_opacity,
                                          comp_op_);
                    break;
                case FT_PIXEL_MODE_MONO:
                    composite_bitmap_mono(pixmap_,
                                     &bit->bitmap,
                                     fill,
                                     x, y,
                                     text_opacity,
                                     comp_op_);
                    break;
            }
        }
        else
        {
            FT_Render_Mode mode = (glyph.info.format.text_mode == TEXT_MODE_DEFAULT)
                ? FT_RENDER_MODE_NORMAL : FT_RENDER_MODE_MONO;
            error = FT_Glyph_To_Bitmap(&glyph.image, mode, 0, 1);
            if (error == 0)
            {
                FT_BitmapGlyph bit = reinterpret_cast<FT_BitmapGlyph>(glyph.image);
                switch (bit->bitmap.pixel_mode)
                {
                    case FT_PIXEL_MODE_GRAY:
                        composite_bitmap(pixmap_,
                                         &bit->bitmap,
                                         fill,
                                         bit->left,
                                         height - bit->top,
                                         text_opacity,
                                         comp_op_,
                                         ras_);
                        break;
                    case FT_PIXEL_MODE_MONO:
                        composite_bitmap_mono(pixmap_,
                                         &bit->bitmap,
                                         fill,
                                         bit->left,
                                         height - bit->top,
                                         text_opacity,
                                         comp_op_);
                        break;
                }
            }
            FT_Done_Glyph(glyph.image);
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
