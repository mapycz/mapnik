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

#ifndef MAPNIK_LABEL_PLACEMENT_HPP
#define MAPNIK_LABEL_PLACEMENT_HPP

#include <mapnik/label_placements/base.hpp>
#include <mapnik/label_placements/point.hpp>
#include <mapnik/label_placements/interior.hpp>
/*
#include <mapnik/label_placements/vertex.hpp>
#include <mapnik/label_placements/line.hpp>
#include <mapnik/label_placements/grid.hpp>
*/
#include <mapnik/symbolizer_enumerations.hpp>

namespace mapnik { namespace label_placement {

struct finder
{
    template <typename Geom>
    static placements_list get(Geom const & geom, label_placement_enum placement_type, placement_params & params)
    {
        switch (placement_type)
        {
            default:
            case POINT_PLACEMENT:
                return point::get(geom, params);
            case INTERIOR_PLACEMENT:
                return interior::get(geom, params);
            /*
            case LINE_PLACEMENT:
                return line.get(geom, params);
            case VERTEX_PLACEMENT:
                return vertex.get(geom, params);
            case GRID_PLACEMENT:
                return grid.get(geom, params);
                */
        }
    }
};

} }

#endif // MAPNIK_LABEL_PLACEMENT_HPP
