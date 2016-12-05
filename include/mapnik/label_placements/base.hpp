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

#include <mapnik/label_collision_detector.hpp>
#include <mapnik/font_engine_freetype.hpp>
#include <mapnik/symbol_cache.hpp>
#include <mapnik/symbolizer_base.hpp>
#include <mapnik/geometry_split_multi.hpp>
#include <mapnik/text/text_layout_generator.hpp>

namespace mapnik { namespace label_placement {

// TODO
struct symbolizer_context
{
    symbolizer_base const & symbolizer;
    feature_impl const & feature;
    attributes const & vars;

    template <typename T>
    T get()
    {
        return mapnik::get<T, mapnik::keys>(symbolizer, feature, vars);
    }
};

template <
    typename LayoutGenerator,
    typename DetectorT = label_collision_detector4>
struct placement_params
{
    using layout_generator_type = LayoutGenerator;

    DetectorT & detector;
    LayoutGenerator & layout_generator;
    mapnik::proj_transform const & proj_transform;
    mapnik::view_transform const & view_transform;
    agg::trans_affine const & affine_transform;
    symbolizer_base const & symbolizer;
    feature_impl const & feature;
    attributes const & vars;
    box2d<double> dims;
    box2d<double> const & query_extent;
    double scale_factor;
    mapnik::symbol_cache const & symbol_cache;
};

// TODO
struct largest_bbox_first
{
    using geom_type = geometry::cref_geometry<double>::geometry_type;

    bool operator() (geom_type const & g0, geom_type const & g1) const
    {
        box2d<double> b0 = geometry::envelope(g0);
        box2d<double> b1 = geometry::envelope(g1);
        return b0.width() * b0.height() < b1.width() * b1.height();
    }
};

template <typename Geom, typename SplitGeoms>
void split(Geom const & geom, SplitGeoms & split_geoms, bool largest_box_only)
{
    geometry::split(geom, split_geoms);
    if (largest_box_only && split_geoms.size() > 1)
    {
        auto type = geometry::geometry_type(geom);
        if (type == geometry::geometry_types::Polygon ||
            type == geometry::geometry_types::MultiPolygon)
        {
            largest_bbox_first compare;
            auto max_it(std::max_element(split_geoms.begin(), split_geoms.end(), compare));
            if (max_it != split_geoms.end())
            {
                auto max(*max_it);
                split_geoms.clear();
                split_geoms.push_back(max);
            }
        }
    }
}

template <typename GeomVisitor, typename Geom>
static std::list<pixel_position> get_pixel_positions(
    Geom const & geom,
    proj_transform const & prj_trans,
    view_transform const & view_trans,
    bool largest_box_only)
{
    using geom_type = geometry::cref_geometry<double>::geometry_type;
    std::vector<geom_type> split_geoms;
    split(geom, split_geoms, largest_box_only);

    std::list<pixel_position> positions;

    for (auto const & geom_ref : split_geoms)
    {
        const GeomVisitor visitor;
        if (boost::optional<geometry::point<double>> point = util::apply_visitor(visitor, geom_ref))
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

struct layout_processor
{
    template <
        typename Geoms,
        typename Layout,
        typename LayoutGenerator,
        typename PlacementsType>
    static void process(
        Geoms & geoms,
        Layout & layout,
        LayoutGenerator & layout_generator,
        PlacementsType & placements)
    {
        while (!geoms.empty() && layout_generator.next())
        {
            for (auto it = geoms.begin(); it != geoms.end(); )
            {
                if (layout.try_placement(layout_generator, *it))
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

} }

#endif // MAPNIK_LABEL_PLACEMENT_BASE_HPP
