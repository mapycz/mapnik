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

#ifndef MAPNIK_LABEL_PLACEMENTS_POINT_HPP
#define MAPNIK_LABEL_PLACEMENTS_POINT_HPP

#include <mapnik/geom_util.hpp>
#include <mapnik/geometry_type.hpp>
#include <mapnik/geometry_centroid.hpp>
#include <mapnik/vertex_adapters.hpp>

namespace mapnik { namespace label_placement {

struct point
{
    static placements_list get(placement_params & params)
    {
        auto const & geom = params.feature.get_geometry();
        placements_list placements;
        auto type = geometry::geometry_type(geom);

        geometry::point<double> pt;
        if (type == geometry::geometry_types::Point)
        {
            pt = mapnik::util::get<geometry::point<double>>(geom);
        }
        else if (type == geometry::geometry_types::LineString)
        {
            auto const & line = mapnik::util::get<geometry::line_string<double>>(geom);
            geometry::line_string_vertex_adapter<double> va(line);
            if (!label::middle_point(va, pt.x, pt.y))
            {
                MAPNIK_LOG_ERROR(label_point_placement) << "Middle point calculation failed.";
                return placements;
            }
        }
        else if (!geometry::centroid(geom, pt))
        {
            MAPNIK_LOG_ERROR(label_point_placement) << "Centroid calculation failed.";
            return placements;
        }

        double z = 0;
        params.proj_transform.backward(pt.x, pt.y, z);
        params.view_transform.forward(&pt.x, &pt.y);

        text_placement_info_ptr info_ptr = mapnik::get<text_placements_ptr>(
            params.symbolizer, keys::text_placements_)->get_placement_info(
                params.scale_factor, params.feature, params.vars, params.symbol_cache);

        placement_finder finder(params.feature, params.vars, params.detector,
            params.dims, *info_ptr, params.font_manager, params.scale_factor);

        pixel_position pos(pt.x, pt.y);

        while (finder.next_position())
        {
            if (finder.find_point_placement(pos))
            {
                placements.emplace_back(std::move(finder.layouts_));
                return placements;
            }
        }
        return placements;
    }
};

} }

#endif // MAPNIK_LABEL_PLACEMENTS_POINT_HPP
