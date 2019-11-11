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

// freetype2
extern "C"
{
#include <ft2build.h>
#include FT_FREETYPE_H
#include FT_STROKER_H
}

namespace mapnik
{

struct rasterizer;

void composite_bitmap(
    image_rgba8 & dst,
    FT_Bitmap *src,
    unsigned rgba,
    int x,
    int y,
    double opacity,
    composite_mode_e comp_op,
    rasterizer const & ras);

void composite_bitmap(
    image_gray8 & dst,
    FT_Bitmap *src,
    int x,
    int y);

void composite_bitmap_mono(
    image_rgba8 & dst,
    FT_Bitmap *src,
    unsigned rgba,
    int x,
    int y,
    double opacity,
    composite_mode_e comp_op);

}

void composite_bitmap_mono(
    image_gray8 & dst,
    FT_Bitmap *src,
    int x,
    int y);

}

#endif // MAPNIK_TEXT_COMPOSITING_HPP
