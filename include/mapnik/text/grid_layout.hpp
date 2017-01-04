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
#ifndef MAPNIK_TEXT_GRID_LAYOUT_HPP
#define MAPNIK_TEXT_GRID_LAYOUT_HPP

#include <mapnik/box2d.hpp>
#include <mapnik/pixel_position.hpp>
#include <mapnik/text/text_layout.hpp>
#include <mapnik/util/noncopyable.hpp>
#include <mapnik/vertex_cache.hpp>
#include <mapnik/text/text_layout_generator.hpp>
#include <mapnik/label_placements/base.hpp>

namespace mapnik
{

template <template <typename, typename> class GridVertexAdapter, typename SubLayout>
class grid_layout : util::noncopyable
{
public:
    using params_type = label_placement::placement_params;

    grid_layout(params_type const & params)
        : sublayout_(params),
          params_(params)
    {
    }

    template <typename LayoutGenerator, typename Path>
    bool try_placement(
        LayoutGenerator & layout_generator,
        Path & path)
    {
        evaluated_text_properties const & text_props = layout_generator.get_text_props();
        double dx = text_props.grid_cell_width * params_.scale_factor;
        double dy = text_props.grid_cell_height * params_.scale_factor;
        return try_placement(layout_generator, path, dx, dy);
    }

protected:
    template <typename LayoutGenerator, typename Path>
    bool try_placement(
        LayoutGenerator & layout_generator,
        Path & path,
        double dx, double dy)
    {
        bool success = false;
        GridVertexAdapter<Path, double> gpa(path, dx, dy);
        pixel_position point;

        for (unsigned cmd; (cmd = gpa.vertex(&point.x, &point.y)) != SEG_END; )
        {
            success |= sublayout_.try_placement(layout_generator, point);
        }

        return success;
    }

    SubLayout sublayout_;
    params_type const & params_;
};

}//ns mapnik

#endif // MAPNIK_TEXT_GRID_LAYOUT_HPP
