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

#ifndef MAPNIK_LABEL_PLACEMENTS_POINT_HPP
#define MAPNIK_LABEL_PLACEMENTS_POINT_HPP

#include <mapnik/geom_util.hpp>
#include <mapnik/geometry_type.hpp>
#include <mapnik/geometry_centroid.hpp>
#include <mapnik/vertex_adapters.hpp>
#include <mapnik/text/point_layout.hpp>

namespace mapnik { namespace label_placement {

struct point
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
            if (!geometry::centroid(poly, pt))
            {
                MAPNIK_LOG_ERROR(label_interior_placement) << "Centroid point calculation failed.";
                return boost::none;
            }
            return pt;
        }
    };

    static placements_list get(placement_params & params)
    {
        //placement_finder & finder = params.placement_finder;
        std::list<pixel_position> points(get_pixel_positions<geometry_visitor>(
            params.feature.get_geometry(),
            params.proj_transform,
            params.view_transform));

        placements_list placements;


        text_placement_info_ptr placement_info = mapnik::get<text_placements_ptr>(
            sym, keys::text_placements_)->get_placement_info(scale_factor,
                feature, vars, sc);

        while (!points.empty() && placement_info.next())
        {
            point_layout layout(params.feature, params.vars, params.detector,
                params.query_extent, placement_info, params.font_manager, params.scale_factor);

            for (auto it = points.begin(); it != points.end(); )
            {
                if (finder.find_point_placement(*it))
                {
                    it = points.erase(it);
                }
                else
                {
                    ++it;
                }
            }

            if (!finder.layouts_->placements_.empty())
            {
TODO:
                placements.emplace_back(std::move(finder.layouts_));
            }
        }

        return placements;
    }
};

} }

#endif // MAPNIK_LABEL_PLACEMENTS_POINT_HPP
