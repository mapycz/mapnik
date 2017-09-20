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

#include <mapnik/util/math.hpp>
#include <mapnik/text/glyph_bbox.hpp>

namespace mapnik
{

box2d<double> get_bbox(
    text_layout const& layout,
    glyph_info const& glyph,
    pixel_position const& pos,
    rotation const& rot)
{
    /*
      (0/ymax)           (width/ymax)
      ***************
      *             *
 (0/0)*             *
      *             *
      ***************
      (0/ymin)          (width/ymin)
      Add glyph offset in y direction, but not in x direction (as we use the full cluster width anyways)!
    */
    double width = layout.cluster_width(glyph.char_index);
    if (glyph.advance() <= 0) width = -width;
    pixel_position tmp, tmp2;
    tmp.set(0, glyph.ymax());
    tmp = tmp.rotate(rot);
    tmp2.set(width, glyph.ymax());
    tmp2 = tmp2.rotate(rot);
    box2d<double> bbox(tmp.x,  -tmp.y,
                       tmp2.x, -tmp2.y);
    tmp.set(width, glyph.ymin());
    tmp = tmp.rotate(rot);
    bbox.expand_to_include(tmp.x, -tmp.y);
    tmp.set(0, glyph.ymin());
    tmp = tmp.rotate(rot);
    bbox.expand_to_include(tmp.x, -tmp.y);
    pixel_position pos2 = pos + pixel_position(0, glyph.offset.y).rotate(rot);
    bbox.move(pos2.x , -pos2.y);
    return bbox;
}

}// ns mapnik
