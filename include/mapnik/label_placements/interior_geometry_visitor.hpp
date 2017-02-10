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
#ifndef MAPNIK_LABEL_PLACEMENT_INTERIOR_GEOMETRY_VISITOR_HPP
#define MAPNIK_LABEL_PLACEMENT_INTERIOR_GEOMETRY_VISITOR_HPP

#include <mapnik/vertex_adapters.hpp>
#include <mapnik/geom_util.hpp>

namespace mapnik { namespace label_placement {

struct interior_geometry_visitor
{
    interior_geometry_visitor()
    {
    }

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

    return_type operator()(geometry::multi_point<double> const & multi) const
    {
        return centroid(multi);
    }

    return_type operator()(geometry::multi_polygon<double> const & multi) const
    {
        return centroid(multi);
    }

    return_type operator()(geometry::multi_line_string<double> const & multi) const
    {
        return centroid(multi);
    }

    template <typename Geom>
    return_type operator()(Geom const & geom) const
    {
        MAPNIK_LOG_WARN(label_interior_placement) << "Trying to find interior position on unsupported geometry";
        return boost::none;
    }

    template <typename Geom>
    return_type centroid(Geom const & geom) const
    {
        geometry::point<double> pt;
        if (!geometry::centroid(geom, pt))
        {
            MAPNIK_LOG_ERROR(label_point_placement) << "Centroid point calculation failed.";
            return boost::none;
        }
        return pt;
    }
};

} }

#endif // MAPNIK_LABEL_PLACEMENT_INTERIOR_GEOMETRY_VISITOR_HPP
