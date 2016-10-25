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

#ifndef MAPNIK_LABEL_PLACEMENTS_INTERIOR_HPP
#define MAPNIK_LABEL_PLACEMENTS_INTERIOR_HPP

#include <mapnik/geom_util.hpp>
#include <mapnik/geometry_types.hpp>
#include <mapnik/geometry_split_multi.hpp>

namespace mapnik { namespace label_placement {

struct interior
{
    template <typename Geom>
    static placements_list get(Geom const & geom, label_placement_params & params)
    {
        placements_list placements;
        auto type = geometry::geometry_type(geom);

        geometry::point<double> pt;
        if (type == geometry::geometry_types::Point)
        {
            pt = geom;
        }
        else if (type == geometry::geometry_types::LineString)
        {
            auto const & line = mapnik::util::get<geometry::line_string<double>>(geom);
            geometry::line_string_vertex_adapter<double> va(line);
            if (!label::middle_point(va, pt.x, pt.y))
            {
                MAPNIK_LOG_ERROR(label_interior_placement) << "Middle point calculation failed." << type_oid;
                return placements;
            }
        }
        else if (type == geometry::geometry_types::Polygon)
        {
            auto const & poly = mapnik::util::get<geometry::polygon<double>>(geom);
            geometry::polygon_vertex_adapter<double> va(poly);
            if (!label::interior_position(va, pt.x, pt.y))
            {
                MAPNIK_LOG_ERROR(label_interior_placement) << "Interior point calculation failed." << type_oid;
                return placements;
            }
        }
        else
        {
            MAPNIK_LOG_ERROR(symbolizer_helpers) << "ERROR: Unknown placement type in initialize_points()";
            return placements;
        }

        double z = 0;
        params.proj_transform.backward(pt.x, pt.y, z);
        params.view_transform.forward(&pt.x, &pt.y);

        placement_finder finder(params.feature, params.vars, params.detector,
            params.dims, params.placement_info, params.font_manager, params.scale_factor);

        pixel_position pos(pt.x, pt.y);

        while (finder.next_position())
        {
            if (finder.find_point_placement(pos))
            {
                return finder.placements();
            }
        }

        return placements;
    }
};

} }

#endif // MAPNIK_LABEL_PLACEMENTS_INTERIOR_HPP
