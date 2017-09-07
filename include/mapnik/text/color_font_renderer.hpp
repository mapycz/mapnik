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

#ifndef MAPNIK_COLOR_FONT_RENDERER_HPP
#define MAPNIK_COLOR_FONT_RENDERER_HPP

#pragma GCC diagnostic push
#include <mapnik/warning_ignore.hpp>

#pragma GCC diagnostic push
#include <mapnik/warning_ignore_agg.hpp>
#include "agg_pixfmt_rgba.h"
#include <agg_trans_affine.h>
#pragma GCC diagnostic pop

// freetype2
extern "C"
{
#include <ft2build.h>
#include FT_FREETYPE_H
#include FT_STROKER_H
}

#pragma GCC diagnostic pop

#include <mapnik/text/glyph_info.hpp>
#include <mapnik/image_any.hpp>
#include <mapnik/image_compositing.hpp>
#include <mapnik/geometry/box2d.hpp>

#include <tuple>
#include <map>

namespace mapnik
{

class halo_cache
{
public:
    using key_type = std::tuple<std::string, // family
                                std::string, // face
                                unsigned, // glyph index
                                unsigned, // glyph height
                                int>; // halo radius
    using img_type = image_gray8;
    using value_type = std::unique_ptr<img_type>;

    using pixfmt_type = agg::pixfmt_rgba32_pre;

    image_gray8 const& get(glyph_info const & glyph,
                           pixfmt_type const& bitmap,
                           double halo_radius);

private:
    std::map<key_type, value_type> cache_;

    void render_halo_img(pixfmt_type const& glyph_bitmap,
                         img_type & halo_bitmap,
                         int radius);
};

template <typename T>
void composite_color_glyph(T & pixmap,
                           FT_Bitmap const& bitmap,
                           agg::trans_affine const& tr,
                           int x,
                           int y,
                           double angle,
                           box2d<double> const& bbox,
                           double opacity,
                           composite_mode_e comp_op);


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
                                halo_cache & cache);

}

#endif
