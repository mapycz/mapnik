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

#ifndef MAPNIK_LABEL_PLACEMENT_VERTEX_HPP
#define MAPNIK_LABEL_PLACEMENT_VERTEX_HPP

#include <mapnik/geom_util.hpp>
#include <mapnik/geometry_types.hpp>
#include <mapnik/vertex_processor.hpp>

namespace mapnik { namespace label_placement {

template <typename Layout, typename LayoutGenerator, typename Detector, typename Placements>
struct vertex
{
    static Placements get(
        LayoutGenerator & layout_generator,
        Detector & detector,
        placement_params const & params)
    {
        using geom_type = geometry::cref_geometry<double>::geometry_type;
        std::list<geom_type> geoms;
        apply_multi_policy(params.feature.get_geometry(), geoms,
            layout_generator.multi_policy());

        Layout layout(params);
        Placements placements;

        layout_processor::process(geoms, layout, layout_generator, detector, placements);

        return placements;
    }
};

} }

#endif // MAPNIK_LABEL_PLACEMENT_VERTEX_HPP
