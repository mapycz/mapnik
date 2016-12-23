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
#include <mapnik/label_placements/vertex.hpp>
#include <mapnik/label_placements/grid.hpp>
#include <mapnik/label_placements/line.hpp>
#include <mapnik/label_placements/vertex_first.hpp>
#include <mapnik/label_placements/vertex_last.hpp>
#include <mapnik/symbolizer_enumerations.hpp>
#include <mapnik/label_collision_detector.hpp>

namespace mapnik { namespace label_placement {

template <typename T>
struct finder
{
    using detector_type = label_collision_detector4;
    using placements_type = typename T::placements_type;
    using layout_generator_type = typename T::layout_generator_type;

    static placements_type get(
        label_placement_enum placement_type,
        layout_generator_type & layout_generator,
        detector_type & detector,
        placement_params const & params)
    {
        switch (placement_type)
        {
            default:
            case POINT_PLACEMENT:
            case CENTROID_PLACEMENT:
                return point<
                    typename T::point,
                    layout_generator_type,
                    detector_type,
                    placements_type>::get(layout_generator, detector, params);
            case INTERIOR_PLACEMENT:
                return interior<
                    typename T::interior,
                    layout_generator_type,
                    detector_type,
                    placements_type>::get(layout_generator, detector, params);
            case VERTEX_PLACEMENT:
                return vertex<
                    typename T::vertex,
                    layout_generator_type,
                    detector_type,
                    placements_type>::get(layout_generator, detector, params);
            case GRID_PLACEMENT:
                return grid<
                    typename T::grid,
                    layout_generator_type,
                    detector_type,
                    placements_type>::get(layout_generator, detector, params);
            case ALTERNATING_GRID_PLACEMENT:
                return grid<
                    typename T::alternating_grid,
                    layout_generator_type,
                    detector_type,
                    placements_type>::get(layout_generator, detector, params);
            case LINE_PLACEMENT:
                return line<
                    typename T::line,
                    layout_generator_type,
                    detector_type,
                    placements_type>::get(layout_generator, detector, params);
            case VERTEX_FIRST_PLACEMENT:
                return vertex_first<
                    typename T::vertex_first,
                    layout_generator_type,
                    detector_type,
                    placements_type>::get(layout_generator, detector, params);
            case VERTEX_LAST_PLACEMENT:
                return vertex_last<
                    typename T::vertex_last,
                    layout_generator_type,
                    detector_type,
                    placements_type>::get(layout_generator, detector, params);
        }
    }
};

} }

#endif // MAPNIK_LABEL_PLACEMENT_HPP
