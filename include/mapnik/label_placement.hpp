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
    using placements_type = typename T::placements_type;
    using layout_generator_type = typename T::layout_generator_type;

    struct dispatch
    {
        layout_generator_type & layout_generator;
        placement_params const & params;

        template <typename Layout>
        bool apply()
        {
            Layout layout(params);
            return layout.try_placement(layout_generator, params);
        }
    };

    static bool apply(
        label_placement_enum placement_type,
        layout_generator_type & layout_generator,
        placement_params const & params)
    {
        dispatch dsp{ layout_generator, params };

        switch (placement_type)
        {
            default:
            case POINT_PLACEMENT:
            case CENTROID_PLACEMENT:
                return dsp.template apply<typename T::point>();
            case INTERIOR_PLACEMENT:
                return dsp.template apply<typename T::interior>();
            case VERTEX_PLACEMENT:
                return dsp.template apply<typename T::vertex>();
            case GRID_PLACEMENT:
                return dsp.template apply<typename T::grid>();
            case ALTERNATING_GRID_PLACEMENT:
                return dsp.template apply<typename T::alternating_grid>();
            case LINE_PLACEMENT:
                return dsp.template apply<typename T::line>();
            case VERTEX_FIRST_PLACEMENT:
                return dsp.template apply<typename T::vertex_first>();
            case VERTEX_LAST_PLACEMENT:
                return dsp.template apply<typename T::vertex_last>();
        }
        return false;
    }
};

} }

#endif // MAPNIK_LABEL_PLACEMENT_HPP
