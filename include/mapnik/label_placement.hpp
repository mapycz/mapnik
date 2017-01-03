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

    struct dispatch
    {
        layout_generator_type & layout_generator;
        detector_type & detector;
        placement_params const & params;

        template <typename Layout>
        bool apply()
        {
            Layout layout(params);
            return layout.try_placement(layout_generator, detector, params);
        }
    };

    static placements_type get(
        label_placement_enum placement_type,
        layout_generator_type & layout_generator,
        detector_type & detector,
        placement_params const & params)
    {
        dispatch dsp{ layout_generator, detector, params };

        switch (placement_type)
        {
            default:
            case POINT_PLACEMENT:
            case CENTROID_PLACEMENT:
                dsp.template apply<typename T::point>();
                break;
            case INTERIOR_PLACEMENT:
                dsp.template apply<typename T::interior>();
                break;
            case VERTEX_PLACEMENT:
                dsp.template apply<typename T::vertex>();
                break;
            case GRID_PLACEMENT:
                dsp.template apply<typename T::grid>();
                break;
            case ALTERNATING_GRID_PLACEMENT:
                dsp.template apply<typename T::alternating_grid>();
                break;
            case LINE_PLACEMENT:
                dsp.template apply<typename T::line>();
                break;
            case VERTEX_FIRST_PLACEMENT:
                dsp.template apply<typename T::vertex_first>();
                break;
            case VERTEX_LAST_PLACEMENT:
                dsp.template apply<typename T::vertex_last>();
        }

        placements_type placements(std::move(layout_generator.placements_));

        return placements;
    }
};

} }

#endif // MAPNIK_LABEL_PLACEMENT_HPP
