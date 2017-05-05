/*****************************************************************************
 *
 * This file is part of Mapnik (c++ mapping toolkit)
 *
 * Copyright (C) 2016 Artem Pavlenko
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
#include <mapnik/text/placement_finder.hpp>
#include <mapnik/extend_converter.hpp>

namespace mapnik
{

template <typename T>
bool placement_finder::find_line_placements(T & path, bool points)
{
    if (!layouts_.line_count()) return true; //TODO

    if (horizontal_alignment_ == H_ADJUST)
    {
        extend_converter<T> ec(path, halign_adjust_extend);
        vertex_cache pp(ec);
        return find_line_placements(pp, points);
    }

    vertex_cache pp(path);
    return find_line_placements(pp, points);
}

}// ns mapnik
