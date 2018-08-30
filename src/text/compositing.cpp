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

#include <mapnik/text/compositing.hpp>
#include <mapnik/color.hpp>
#include <mapnik/safe_cast.hpp>
#include <mapnik/box2d.hpp>

#pragma GCC diagnostic push
#include <mapnik/warning_ignore_agg.hpp>
#include "agg_pixfmt_rgba.h"
#pragma GCC diagnostic pop

namespace mapnik
{

void composite_bitmap_src_over(
    image_rgba8 & dst,
    FT_Bitmap *src,
    unsigned rgba,
    int x,
    int y,
    double opacity)
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
    const unsigned ca = c.alpha();
    const unsigned cb = c.blue();
    const unsigned cg = c.green();
    const unsigned cr = c.red();

    for (int i = box.minx(), p = box.minx() - x; i < box.maxx(); ++i, ++p)
    {
        for (int j = box.miny(), q = box.miny() - y; j < box.maxy(); ++j, ++q)
        {
            unsigned gray = src->buffer[q * src->width + p];
            if (gray)
            {
                image_rgba8::pixel_type & pix = dst(i, j);
                value_type *p = reinterpret_cast<value_type*>(&pix);

                const unsigned sr = (cr * gray + 255) >> 8;
                const unsigned sg = (cg * gray + 255) >> 8;
                const unsigned sb = (cb * gray + 255) >> 8;
                const unsigned sa = (ca * gray + 255) >> 8;

                const color_type::calc_type s1a = color_type::base_mask - sa;

                p[order_type::R] = (value_type)(sr + ((p[order_type::R] * s1a +
                    color_type::base_mask) >> color_type::base_shift));
                p[order_type::G] = (value_type)(sg + ((p[order_type::G] * s1a +
                    color_type::base_mask) >> color_type::base_shift));
                p[order_type::B] = (value_type)(sb + ((p[order_type::B] * s1a +
                    color_type::base_mask) >> color_type::base_shift));
                p[order_type::A] = (value_type)(sa + ((p[order_type::A] * s1a +
                    color_type::base_mask) >> color_type::base_shift));
            }
        }
    }
}

void composite_bitmap(
    image_rgba8 & dst,
    FT_Bitmap *src,
    unsigned rgba,
    int x,
    int y,
    double opacity,
    composite_mode_e comp_op)
{
    if (comp_op == src_over)
    {
        composite_bitmap_src_over(dst, src, rgba, x, y, opacity);
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
                mapnik::composite_pixel(dst, comp_op, i, j, rgba, gray, opacity);
            }
        }
    }
}

}
