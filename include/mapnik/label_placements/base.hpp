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

#ifndef MAPNIK_LABEL_PLACEMENT_BASE_HPP
#define MAPNIK_LABEL_PLACEMENT_BASE_HPP

#include <mapnik/symbol_cache.hpp>
#include <mapnik/symbolizer_base.hpp>
#include <mapnik/symbolizer.hpp>
#include <mapnik/geometry_split_multi.hpp>
#include <mapnik/view_transform.hpp>
#include <mapnik/pixel_position.hpp>

#include <list>

namespace mapnik { namespace label_placement {

struct placement_params
{
    mapnik::proj_transform const & proj_transform;
    mapnik::view_transform const & view_transform;
    agg::trans_affine const & affine_transform;
    symbolizer_base const & symbolizer;
    feature_impl const & feature;
    attributes const & vars;
    const box2d<double> dims;
    box2d<double> const & query_extent;
    double scale_factor;
    mapnik::symbol_cache const & symbol_cache;

    template <typename T, mapnik::keys key>
    T get() const
    {
        return mapnik::get<T, key>(symbolizer, feature, vars);
    }

    template <typename T, mapnik::keys key>
    boost::optional<T> get_optional() const
    {
        return mapnik::get_optional<T, key>(symbolizer, feature, vars);
    }
};

template <typename It>
static It largest_bbox(It begin, It end)
{
    if (begin == end)
    {
        return end;
    }
    It largest_geom = begin;
    double largest_bbox = geometry::envelope(*largest_geom).area();
    for (++begin; begin != end; ++begin)
    {
        double bbox = geometry::envelope(*begin).area();
        if (bbox > largest_bbox)
        {
            largest_bbox = bbox;
            largest_geom = begin;
        }
    }
    return largest_geom;
}

template <typename Geom, typename SplitGeoms>
void apply_multi_policy(
    Geom const & geom,
    SplitGeoms & split_geoms,
    multi_policy_enum multi_policy)
{
    switch (multi_policy)
    {
        default:
        case EACH_MULTI:
            geometry::split(geom, split_geoms);
            break;
        case WHOLE_MULTI:
            split_geoms.emplace_back(geometry::to_cref(geom));
            break;
        case LARGEST_MULTI:
        {
            geometry::split(geom, split_geoms);

            if (split_geoms.size() > 1)
            {
                auto type = geometry::geometry_type(geom);
                if (type == geometry::geometry_types::Polygon ||
                    type == geometry::geometry_types::MultiPolygon)
                {
                    auto largest_geom = *largest_bbox(
                        split_geoms.begin(),
                        split_geoms.end());
                    split_geoms.clear();
                    split_geoms.push_back(largest_geom);
                }
            }
        }
    }
}

enum placement_result
{
    success,
    collision,
    skip,
    stop,
};

}

struct point_position
{
    pixel_position coords;
    double angle;
};

}

#endif // MAPNIK_LABEL_PLACEMENT_BASE_HPP
