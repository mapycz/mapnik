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
#ifndef MAPNIK_MARKER_GRID_LAYOUT_HPP
#define MAPNIK_MARKER_GRID_LAYOUT_HPP

#include <mapnik/text/grid_layout.hpp>

namespace mapnik
{

template <typename SubLayout>
class marker_grid_layout : grid_layout<SubLayout>
{
public:
    using params_type = label_placement::placement_params;

    marker_grid_layout(params_type const & params)
        : grid_layout<SubLayout>(params),
          dx_(params.get<value_double, keys::grid_cell_width>()),
          dy_(params.get<value_double, keys::grid_cell_height>())
    {
    }

    template <typename LayoutGenerator, typename Detector, typename Geom>
    bool try_placement(
        LayoutGenerator & layout_generator,
        Detector & detector,
        Geom const & geom)
    {
        return grid_layout<SubLayout>::try_placement(layout_generator, detector, geom, dx_, dy_);
    }

protected:
    const double dx_, dy_;
};

}//ns mapnik

#endif // MAPNIK_MARKER_GRID_LAYOUT_HPP
