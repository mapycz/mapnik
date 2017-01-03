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
#include <mapnik/symbolizer_enumerations.hpp>
#include <mapnik/collision_cache.hpp>

namespace mapnik { namespace label_placement {

template <typename T>
struct finder
{
    using detector_type = keyed_collision_cache<label_collision_detector4>;
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
            {
                typename T::point layout(params);
                layout.try_placement(layout_generator, detector, params);
            }
            break;
            case INTERIOR_PLACEMENT:
            {
                typename T::interior layout(params);
                layout.try_placement(layout_generator, detector, params);
            }
            break;
            case VERTEX_PLACEMENT:
            {
                typename T::vertex layout(params);
                layout.try_placement(layout_generator, detector, params);
            }
            break;
            case GRID_PLACEMENT:
            {
                typename T::grid layout(params);
                layout.try_placement(layout_generator, detector, params);
            }
            break;
            case ALTERNATING_GRID_PLACEMENT:
            {
                typename T::alternating_grid layout(params);
                layout.try_placement(layout_generator, detector, params);
            }
            break;
            case LINE_PLACEMENT:
            {
                typename T::line layout(params);
                layout.try_placement(layout_generator, detector, params);
            }
            break;
            case VERTEX_FIRST_PLACEMENT:
            {
                typename T::vertex_first layout(params);
                layout.try_placement(layout_generator, detector, params);
            }
            break;
            case VERTEX_LAST_PLACEMENT:
            {
                typename T::vertex_last layout(params);
                layout.try_placement(layout_generator, detector, params);
            }
        }

        placements_type placements(std::move(layout_generator.placements_));

        return placements;
    }
};

} }

#endif // MAPNIK_LABEL_PLACEMENT_HPP
