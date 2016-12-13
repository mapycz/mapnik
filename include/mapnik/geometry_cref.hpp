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

#ifndef MAPNIK_GEOMETRY_CREF_HPP
#define MAPNIK_GEOMETRY_CREF_HPP

#include <mapnik/geometry.hpp>
#include <mapnik/geometry_type.hpp>
#include <mapnik/geometry_envelope.hpp>
#include <mapnik/box2d.hpp>

#include <functional>

namespace mapnik { namespace geometry {

template <typename T>
struct cref_geometry
{
    using empty = std::reference_wrapper<geometry_empty const>;
    using point_type = std::reference_wrapper<point<T> const>;
    using line_string_type = std::reference_wrapper<line_string<T> const>;
    using polygon_type = std::reference_wrapper<polygon<T> const>;
    using multi_point_type = std::reference_wrapper<multi_point<T> const>;
    using multi_line_string_type = std::reference_wrapper<multi_line_string<T> const>;
    using multi_polygon_type = std::reference_wrapper<multi_polygon<T> const>;
    using geometry_type = util::variant<
        empty,
        point_type,
        line_string_type,
        polygon_type,
        multi_point_type,
        multi_line_string_type,
        multi_polygon_type>;
};

template <typename T>
struct cref_geometry_cast
{
    using geometry_type = typename cref_geometry<T>::geometry_type;

    template <typename Geometry>
    geometry_type operator() (Geometry const& geom) const
    {
        return std::cref(geom);
    }
};

template <typename Geom, typename T = double>
inline typename cref_geometry<T>::geometry_type to_cref(Geom const & geom)
{
    cref_geometry_cast<T> visitor;
    return util::apply_visitor(visitor, geom);
}

struct envelope_impl
{
    template <typename T>
    box2d<double> operator() (T const& ref) const
    {
        return envelope<T>(ref);
    }
};

mapnik::box2d<double> envelope(cref_geometry<double>::geometry_type const& geom);

struct geometry_type_impl
{
    template <typename T>
    auto operator() (T const& ref) const -> decltype(geometry_type<T>(ref))
    {
        return geometry_type<T>(ref);
    }
};

mapnik::geometry::geometry_types geometry_type(cref_geometry<double>::geometry_type const& geom);

} }

#endif // MAPNIK_GEOMETRY_CREF_HPP
