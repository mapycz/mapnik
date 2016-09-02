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

#ifndef MAPNIK_MARKERS_PLACEMENTS_POINT_HPP
#define MAPNIK_MARKERS_PLACEMENTS_POINT_HPP

#include <mapnik/geom_util.hpp>
#include <mapnik/geometry_types.hpp>
#include <mapnik/geometry_split_multi.hpp>

namespace mapnik {

template <typename Geom, typename Detector>
class markers_point_placement
{
public:
    markers_point_placement(Geom & geom, Detector & detector,
                            markers_placement_params const& params)
        : markers_basic_placement(params),
          geom_(geom),
          detector_(detector),
          done_(false)
    {
    }

    placements_list get(label_placement_params & params)
    {
        using container_type = geometry::split_multi_geometries<double, std::list>::container_type;
        container_type geoms;
        geometry::split<double>(params.feature.get_geometry(), geoms);

        placement_finder finder(params.feature, params.vars, params.detector,
            params.dims, params.placement_info, params.font_manager, params.scale_factor);
    }

protected:
    Geom & geom_;
    Detector & detector_;
    bool done_;
};

}

#endif // MAPNIK_MARKERS_PLACEMENTS_POINT_HPP
