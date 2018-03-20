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

#ifndef MAPNIK_FAST_COMP_OP
#define MAPNIK_FAST_COMP_OP

#include <mapnik/config.hpp>
#include <mapnik/image.hpp>
#include <mapnik/box2d.hpp>

namespace mapnik {

void composite_src_over(image_rgba8 const& src,
                        image_rgba8 & dst,
                        int x,
                        int y,
                        float opacity)
{
    mapnik::box2d<int> ext0(0, 0, dst.width(), dst.height());
    mapnik::box2d<int> ext1(x, y, x + src.width(), y + src.height());

    if (ext0.intersects(ext1))
    {
        mapnik::box2d<int> box = ext0.intersect(ext1);
        for (std::size_t pix_y = box.miny(); pix_y < static_cast<std::size_t>(box.maxy()); ++pix_y)
        {
            typename image_rgba8::pixel_type * row_to =  dst.get_row(pix_y);
            typename image_rgba8::pixel_type const * row_from = src.get_row(pix_y - y);

            for (std::size_t pix_x = box.minx(); pix_x < static_cast<std::size_t>(box.maxx()); pix_x+=4)
            {
                row_to[pix_x] = row_from[pix_x - x];
            }
        }
    }
}

} // end ns mapnik

#endif // MAPNIK_FAST_COMP_OP
