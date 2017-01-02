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

/*
template <typename GeomVisitor, typename Geoms>
static std::list<pixel_position> get_pixel_positions(
    Geoms const & geoms,
    proj_transform const & prj_trans,
    view_transform const & view_trans)
{
    std::list<pixel_position> positions;
    for (auto const & geom : geoms)
    {
        const GeomVisitor visitor;
        if (boost::optional<geometry::point<double>> point = util::apply_visitor(visitor, geom))
        {
            geometry::point<double> & pt = *point;
            double z = 0;
            prj_trans.backward(pt.x, pt.y, z);
            view_trans.forward(&pt.x, &pt.y);
            positions.emplace_back(pt.x, pt.y);
        }
    }
    return positions;
}
*/

/*
struct layout_processor
{
    template <
        typename Geoms,
        typename Layout,
        typename LayoutGenerator,
        typename Detector,
        typename PlacementsType>
    static void process(
        Geoms & geoms,
        Layout & layout,
        LayoutGenerator & layout_generator,
        Detector & detector,
        PlacementsType & placements)
    {
        while (!geoms.empty() && layout_generator.next())
        {
            for (auto it = geoms.begin(); it != geoms.end(); )
            {
                if (layout.try_placement(layout_generator, detector, *it))
                {
                    it = geoms.erase(it);
                }
                else
                {
                    ++it;
                }
            }

            if (layout_generator.has_placements())
            {
                placements.emplace_back(std::move(layout_generator.get_placements()));
            }
        }
    }
};
*/

}

struct point_position
{
    pixel_position coords;
    double angle;
};

}

#endif // MAPNIK_LABEL_PLACEMENT_BASE_HPP
