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

//mapnik
#include <mapnik/box2d.hpp>
#include <mapnik/pixel_position.hpp>
#include <mapnik/text/text_layout.hpp>
#include <mapnik/text/glyph_positions.hpp>
#include <mapnik/text/rotation.hpp>
#include <mapnik/util/noncopyable.hpp>
#include <mapnik/extend_converter.hpp>
#include <mapnik/vertex_cache.hpp>
#include <mapnik/text/text_layout_generator.hpp>
#include <mapnik/label_placements/base.hpp>
#include <mapnik/vertex_converters.hpp>
#include <mapnik/vertex_adapters.hpp>
#include <mapnik/grid_vertex_adapter.hpp>

namespace mapnik
{

template <typename SubLayout>
class marker_grid_layout : grid_layout<SubLayout>
{
public:
    using params_type = label_placement::placement_params;

    marker_grid_layout(params_type const & params);

    template <typename LayoutGenerator, typename Detector, typename Geom>
    bool try_placement(
        LayoutGenerator & layout_generator,
        Detector & detector,
        Geom const & geom);

protected:
    const double dx_, dy_;
};

template <typename SubLayout>
marker_grid_layout<SubLayout>::marker_grid_layout(params_type const & params)
    : grid_layout<SubLayout>(params),
      dx_(),
      dy_()
{
}

template <typename SubLayout>
template <typename LayoutGenerator, typename Detector, typename Geom>
bool grid_layout<SubLayout>::try_placement(
    LayoutGenerator & layout_generator,
    Detector & detector,
    Geom const & geom_ref)
{
    using polygon_type = geometry::cref_geometry<double>::polygon_type;
    auto const & poly = mapnik::util::get<polygon_type>(geom_ref).get();
    geometry::polygon_vertex_adapter<double> va(poly);
    using positions_type = std::list<pixel_position>;
    positions_type points;
    evaluated_text_properties const & text_props = layout_generator.get_text_props();
    detail::grid_placement_finder_adapter<double, positions_type> ga(
        text_props.grid_cell_width, text_props.grid_cell_height, points);
    converter_.apply(va, ga);

    bool success = false;

    for (auto const & point : points)
    {
        success |= sublayout_.try_placement(layout_generator, detector, point);
    }

    return success;
}

}//ns mapnik

#endif // MAPNIK_MARKER_GRID_LAYOUT_HPP
