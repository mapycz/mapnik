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

#ifndef MAPNIK_LABEL_PLACEMENTS_INTERIOR_HPP
#define MAPNIK_LABEL_PLACEMENTS_INTERIOR_HPP

#include <mapnik/geom_util.hpp>
#include <mapnik/geometry_types.hpp>

namespace mapnik { namespace label_placement {

template <typename Layout>
struct interior
{
    struct geometry_visitor
    {
        using return_type = boost::optional<geometry::point<double>>;

        return_type operator()(geometry::point<double> const & point) const
        {
            return point;
        }

        return_type operator()(geometry::line_string<double> const & line) const
        {
            geometry::point<double> pt;
            geometry::line_string_vertex_adapter<double> va(line);
            if (!label::middle_point(va, pt.x, pt.y))
            {
                MAPNIK_LOG_ERROR(label_interior_placement) << "Middle point calculation failed.";
                return boost::none;
            }
            return pt;
        }

        return_type operator()(geometry::polygon<double> const & poly) const
        {
            geometry::point<double> pt;
            geometry::polygon_vertex_adapter<double> va(poly);
            if (!label::interior_position(va, pt.x, pt.y))
            {
                MAPNIK_LOG_ERROR(label_interior_placement) << "Interior point calculation failed.";
                return boost::none;
            }
            return pt;
        }
    };

    static placements_list get(placement_params & params)
    {
        std::list<pixel_position> points(get_pixel_positions<geometry_visitor>(
            params.feature.get_geometry(),
            params.proj_transform,
            params.view_transform));
        placements_list placements;
        Layout layout(params.detector, params.dims, params.scale_factor,
            params.symbolizer, params.feature, params.vars);

        while (!points.empty() && params.layout_generator.next())
        {
            for (auto it = points.begin(); it != points.end(); )
            {
                if (layout.try_placement(params.layout_generator, *it))
                {
                    it = points.erase(it);
                }
                else
                {
                    ++it;
                }
            }

            if (!params.layout_generator.get_layouts()->placements_.empty())
            {
                placements.emplace_back(std::move(params.layout_generator.get_layouts()));
            }
        }

        return placements;
    }
};

} }

#endif // MAPNIK_LABEL_PLACEMENTS_INTERIOR_HPP
