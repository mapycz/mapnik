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
#ifndef MAPNIK_LABEL_PLACEMENT_POINT_POSITION_ACCESSOR_HPP
#define MAPNIK_LABEL_PLACEMENT_POINT_POSITION_ACCESSOR_HPP

#include <mapnik/label_placements/line_layout.hpp>
#include <mapnik/text/point_layout.hpp>

namespace mapnik { namespace label_placement {

template <>
struct position_accessor<text::point_layout>
{
    static pixel_position const & get(vertex_cache const & geom)
    {
        return geom.current_position();
    }

    static pixel_position const & get(pixel_position const & geom)
    {
        return geom;
    }
};

template <>
struct position_accessor<text::shield_layout> : position_accessor<text::point_layout>
{
};

} }

#endif
