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

#ifndef MAPNIK_TEXT_COMPOSITING_HPP
#define MAPNIK_TEXT_COMPOSITING_HPP

#include <mapnik/config.hpp>
#include <mapnik/image_util.hpp>
#include <mapnik/color.hpp>
#include <mapnik/safe_cast.hpp>
#include <mapnik/box2d.hpp>

#pragma GCC diagnostic push
#include <mapnik/warning_ignore_agg.hpp>
#include "agg_pixfmt_rgba.h"
#pragma GCC diagnostic pop

// freetype2
extern "C"
{
#include <ft2build.h>
#include FT_FREETYPE_H
#include FT_STROKER_H
}

namespace mapnik
{

template <typename GammaFunc>
void composite_bitmap_src_over(
    image_rgba8 & dst,
    FT_Bitmap *src,
    unsigned rgba,
    int x,
    int y,
    double opacity,
    GammaFunc const & gamma_func)
{
    box2d<int> dst_ext(0, 0, dst.width(), dst.height());
    box2d<int> src_ext(x, y, x + src->width, y + src->rows);

    if (!dst_ext.intersects(src_ext))
    {
        return;
    }

    box2d<int> box = dst_ext.intersect(src_ext);

    using color_type = agg::rgba8;
    using value_type = color_type::value_type;
    using order_type = agg::order_rgba;
    using blender_type = agg::comp_op_rgba_src_over<color_type, order_type>;

    color c(rgba);
    c.set_alpha(safe_cast<unsigned>(c.alpha() * opacity));
    c.premultiply();
    unsigned ca = c.alpha();
    unsigned cb = c.blue();
    unsigned cg = c.green();
    unsigned cr = c.red();

    for (int i = box.minx(), p = box.minx() - x; i < box.maxx(); ++i, ++p)
    {
        for (int j = box.miny(), q = box.miny() - y; j < box.maxy(); ++j, ++q)
        {
            unsigned gray = src->buffer[q * src->width + p];
            if (gray)
            {
                image_rgba8::pixel_type & pix = dst(i, j);
                blender_type::blend_pix(reinterpret_cast<value_type*>(&pix),
                                        cr, cg, cb, ca, gamma_func(gray));
            }
        }
    }
}

template <typename GammaFunc>
void composite_bitmap(
    image_rgba8 & dst,
    FT_Bitmap *src,
    unsigned rgba,
    int x,
    int y,
    double opacity,
    composite_mode_e comp_op,
    GammaFunc const & gamma_func)
{
    if (comp_op == src_over)
    {
        composite_bitmap_src_over(dst, src,
            rgba, x, y, opacity, gamma_func);
        return;
    }

    int x_max = x + src->width;
    int y_max = y + src->rows;

    for (int i = x, p = 0; i < x_max; ++i, ++p)
    {
        for (int j = y, q = 0; j < y_max; ++j, ++q)
        {
            unsigned gray = src->buffer[q * src->width + p];
            if (gray)
            {
                mapnik::composite_pixel(dst, comp_op, i, j,
                    rgba, gamma_func(gray), opacity);
            }
        }
    }
}

}

#endif // MAPNIK_TEXT_COMPOSITING_HPP
