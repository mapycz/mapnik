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
//mapnik
#include <mapnik/debug.hpp>
#include <mapnik/label_collision_detector.hpp>
#include <mapnik/view_transform.hpp>
#include <mapnik/expression_evaluator.hpp>
#include <mapnik/text/grid_layout.hpp>
#include <mapnik/text/placements/base.hpp>
#include <mapnik/text/text_layout.hpp>
#include <mapnik/text/glyph_info.hpp>
#include <mapnik/text/text_properties.hpp>
#include <mapnik/text/glyph_positions.hpp>
#include <mapnik/vertex_cache.hpp>
#include <mapnik/util/math.hpp>
#include <mapnik/symbolizer.hpp>
#include <mapnik/marker.hpp>
#include <mapnik/grid_vertex_adapter.hpp>

// stl
#include <vector>

namespace mapnik
{

template <typename T, typename Points>
struct grid_placement_finder_adapter
{
    grid_placement_finder_adapter(T dx, T dy, Points & points)
        : dx_(dx), dy_(dy),
          points_(points) {}

    template <typename PathT>
    void add_path(PathT & path) const
    {
        geometry::grid_vertex_adapter<PathT, T> gpa(path, dx_, dy_);
        gpa.rewind(0);
        double label_x, label_y;
        for (unsigned cmd; (cmd = gpa.vertex(&label_x, &label_y)) != SEG_END; )
        {
            points_.emplace_back(label_x, label_y);
        }
    }

    T dx_, dy_;
    Points & points_;
};

template <typename SubLayout>
grid_layout<SubLayout>::grid_layout(placement_params const & params)
    : sublayout_(params),
      params_(params)
{
}

template <typename SubLayout> template <typename Geom>
bool grid_layout<SubLayout>::try_placement(
    text_layout_generator & layout_generator,
    Geom & geom)
{
    geometry::polygon_vertex_adapter<double> va(geom);
    using positions_type = std::list<pixel_position>;
    positions_type points;
    evaluated_text_properties const & text_props = layout_generator.get_text_props();
    grid_placement_finder_adapter<double, positions_type> ga(
        text_props.grid_cell_width, text_props.grid_cell_height, points);
    converter.apply(va, ga);

    bool success = false;

    for (auto const & point : points)
    {
        success |= sublayout_.try_placement(layout_generator, point);
    }

    return success;
}

} // ns mapnik
