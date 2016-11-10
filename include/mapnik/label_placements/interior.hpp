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
#include <mapnik/geometry_split_multi.hpp>

namespace mapnik { namespace label_placement {

struct interior
{
    using point_type = geometry::point<double>;

    struct geometry_visitor
    {
        using cref_geom_type = geometry::cref_geometry<double>;

        boost::optional<point_type> operator()(cref_geom_type::point_type const & point) const
        {
            return point.get();
        }

        boost::optional<point_type> operator()(cref_geom_type::line_string_type const & line) const
        {
            point_type pt;
            geometry::line_string_vertex_adapter<double> va(line.get());
            if (!label::middle_point(va, pt.x, pt.y))
            {
                MAPNIK_LOG_ERROR(label_interior_placement) << "Middle point calculation failed.";
                return boost::none;
            }
            return pt;
        }

        boost::optional<point_type> operator()(cref_geom_type::polygon_type const & poly) const
        {
            point_type pt;
            geometry::polygon_vertex_adapter<double> va(poly.get());
            if (!label::interior_position(va, pt.x, pt.y))
            {
                MAPNIK_LOG_ERROR(label_interior_placement) << "Interior point calculation failed.";
                return boost::none;
            }
            return pt;
        }

        template <typename T>
        boost::optional<point_type> operator()(T const & geom) const
        {
            MAPNIK_LOG_ERROR(symbolizer_helpers) << "ERROR: Unknown placement type in initialize_points()";
            return boost::none;
        }
    };

    template <typename Geom>
    static boost::optional<pixel_position> get_pixel_position(
        Geom const & geom,
        proj_transform const & prj_trans,
        view_transform const & view_trans)
    {
        geometry_visitor visitor;
        if (boost::optional<point_type> point = util::apply_visitor(visitor, geom))
        {
            point_type & pt = *point;
            double z = 0;
            prj_trans.backward(pt.x, pt.y, z);
            view_trans.forward(&pt.x, &pt.y);
            return pixel_position(pt.x, pt.y);
        }
        return boost::none;
    }

    static placements_list get(placement_params & params)
    {
        text_placement_info_ptr info_ptr = mapnik::get<text_placements_ptr>(
            params.symbolizer, keys::text_placements_)->get_placement_info(
                params.scale_factor, params.feature, params.vars, params.symbol_cache);

        placement_finder finder(params.feature, params.vars, params.detector,
            params.dims, *info_ptr, params.font_manager, params.scale_factor);

        using geom_type = geometry::cref_geometry<double>::geometry_type;
        std::vector<geom_type> splitted;
        geometry::split(params.feature.get_geometry(), splitted);

        placements_list placements;

        for (auto const & geom_ref : splitted)
        {
            boost::optional<pixel_position> pos(get_pixel_position(geom_ref, params.proj_transform, params.view_transform));
            if (pos)
            {
                while (finder.next_position())
                {
                    if (finder.find_point_placement(*pos))
                    {
                        placements.emplace_back(std::move(finder.layouts_));
                        return placements;
                    }
                }
            }
        }

        return placements;
    }
};

} }

#endif // MAPNIK_LABEL_PLACEMENTS_INTERIOR_HPP
