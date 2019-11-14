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
#include <mapnik/text/glyph_cache.hpp>
#include <mapnik/text/compositing.hpp>
#include <mapnik/text/text_properties.hpp>
#include <mapnik/font_engine_freetype.hpp>
#include <mapnik/image_compositing.hpp>
#include <mapnik/image_scaling.hpp>
#include <mapnik/text/face.hpp>
#include <mapnik/image_util.hpp>
#include <mapnik/pixel_position.hpp>
#include <mapnik/symbolizer_enumerations.hpp>

namespace mapnik
{

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

const glyph_cache::value_type * glyph_cache::get(glyph_info const & glyph, double halo_radius)
{
    //std::clog << glyph.face->family_name << "; " << glyph.face->style_name << std::endl;
    std::lock_guard<std::mutex> lock(mutex_);
    glyph_cache_key key{*glyph.face, glyph.glyph_index, glyph.format.text_size, halo_radius};

    if (auto it = cache_.find(key); it != cache_.end())
    {
        return &it->second;
    }

    return render(key, glyph, halo_radius * scale);
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
    int scaled_size = static_cast<int>(glyph.format.text_size * scale);
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

const glyph_cache::value_type * glyph_cache::render(
    glyph_cache_key const & key,
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
        glyph.face->set_character_sizes(glyph.format.text_size * scale);
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
        auto result = cache_.emplace(
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
        if (halo_radius > 0.0)
        {
            stroker_.init(halo_radius);
            FT_Glyph_Stroke(&g, stroker_.get(), 1);
        }
        if (!FT_Glyph_To_Bitmap(&g, FT_RENDER_MODE_NORMAL, 0, 1))
        {
            FT_BitmapGlyph bit = reinterpret_cast<FT_BitmapGlyph>(g);
            auto result = cache_.emplace(
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

const glyph_metrics * glyph_cache::metrics(glyph_metrics_cache_key const & key)
{
    std::lock_guard<std::mutex> lock(mutex_);

    auto it = metrics_cache_.find(key);
    if (it != metrics_cache_.end()) {
        return &it->second;
    }

    glyph_metrics new_metrics;
    key.face.glyph_dimensions(key.glyph_index, TEXT_MODE_DEFAULT, new_metrics);
    auto result = metrics_cache_.emplace(key, new_metrics);
    return result.second ? &result.first->second : nullptr;
}

} // namespace mapnik
