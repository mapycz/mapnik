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

#ifndef MAPNIK_GEOMETRY_SPLIT_MULTI_HPP
#define MAPNIK_GEOMETRY_SPLIT_MULTI_HPP

#include <mapnik/geometry_cref.hpp>

namespace mapnik { namespace geometry {

template <typename T, typename Container>
struct split_multi_geometries
{
    using geometry_type = typename cref_geometry<T>::geometry_type;

    split_multi_geometries(Container & cont)
        : cont_(cont) { }

    void operator() (geometry_empty const&) const {}
    void operator() (multi_point<T> const& multi_pt) const
    {
        for (auto const& pt : multi_pt)
        {
            cont_.emplace_back(std::cref(pt));
        }
    }
    void operator() (line_string<T> const& line) const
    {
        cont_.emplace_back(std::cref(line));
    }

    void operator() (multi_line_string<T> const& multi_line) const
    {
        for (auto const& line : multi_line)
        {
            (*this)(line);
        }
    }

    void operator() (polygon<T> const& poly) const
    {
        cont_.emplace_back(std::cref(poly));
    }

    void operator() (multi_polygon<T> const& multi_poly) const
    {
        for (auto const& poly : multi_poly)
        {
            (*this)(poly);
        }
    }

    void operator() (geometry_collection<T> const& collection) const
    {
        for (auto const& geom : collection)
        {
            util::apply_visitor(*this, geom);
        }
    }

    template <typename Geometry>
    void operator() (Geometry const& geom) const
    {
        cont_.emplace_back(std::cref(geom));
    }

    Container & cont_;
};

template <typename Geom, typename Container, typename T = double>
inline void split(Geom const & geom, Container & container)
{
    split_multi_geometries<T, Container> visitor(container);
    util::apply_visitor(visitor, geom);
}

}}

#endif // MAPNIK_GEOMETRY_SPLIT_MULTI_HPP
