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

template <typename Layout, typename LayoutGenerator, typename Detector, typename Placements>
struct vertex_first
{
    struct point_position
    {
        double x, y;
        double angle;
    };

    template <typename Geom>
    boost::optional<point_position> get_first_vertext(Geom & geom)
    {
        point_position pos;

        if (agg::is_stop(geom.vertex(&pos.x, &pos.y)))
        {
            this->done_ = true;
            return boost::none;
        }

        pos.angle = 0;

        double x1, y1;

        if (agg::is_line_to(geom.vertex(&x1, &y1)))
        {
            pos.angle = std::atan2(y1 - pos.y, x1 - pos.x);
        }

        return pos;
    }

    template <typename Visitor>
    struct layout_adapter
    {
        layout_adapter(Visitor & visitor)
            : visitor_(visitor)
        {
        }

        template <typename Geom>
        bool try_placement(
            LayoutGenerator & generator,
            Detector & detector,
            Geom const & geom)
        {
            return util::apply_visitor(visitor_, geom);
        }

        Visitor visitor_;
    };

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
                MAPNIK_LOG_ERROR(label_point_placement) << "Middle point calculation failed.";
                return boost::none;
            }
            return pt;
        }

        return_type operator()(geometry::polygon<double> const & poly) const
        {
            return centroid(poly);
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
            MAPNIK_LOG_WARN(label_point_placement) << "Trying to find point position on unsupported geometry";
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

    using vertex_converter_type = vertex_converter<
        clip_line_tag,
        clip_poly_tag,
        transform_tag,
        affine_transform_tag,
        extend_tag,
        simplify_tag,
        smooth_tag>;

    static Placements get(
        LayoutGenerator & layout_generator,
        Detector & detector,
        placement_params const & params)
    {
        vertex_converter_type converter(params.query_extent, params.symbolizer,
            params.view_transform, params.proj_transform, params.affine_transform,
            params.feature, params.vars, params.scale_factor);

        value_bool clip = mapnik::get<value_bool, keys::clip>(params.symbolizer, params.feature, params.vars);
        value_double simplify_tolerance = mapnik::get<value_double, keys::simplify_tolerance>(params.symbolizer, params.feature, params.vars);
        value_double smooth = mapnik::get<value_double, keys::smooth>(params.symbolizer, params.feature, params.vars);
        value_double extend = mapnik::get<value_double, keys::extend>(params.symbolizer, params.feature, params.vars);

        if (clip) converter.template set<clip_line_tag>();
        converter.template set<transform_tag>();
        converter.template set<affine_transform_tag>();
        if (extend > 0.0) converter.template set<extend_tag>();
        if (simplify_tolerance > 0.0) converter.template set<simplify_tag>();
        if (smooth > 0.0) converter.template set<smooth_tag>();

        Layout layout(params);
        Placements placements;

        using geom_type = geometry::cref_geometry<double>::geometry_type;
        std::list<geom_type> geoms;
        apply_multi_policy(params.feature.get_geometry(), geoms,
            layout_generator.multi_policy());

        using adapter_type = placement_finder_adapter<Layout, LayoutGenerator, Detector>;
        using visitor_type = line_placement_visitor<adapter_type, vertex_converter_type>;
        adapter_type adapter(layout, layout_generator, detector);
        visitor_type visitor(converter, adapter);
        layout_adapter<visitor_type> la(visitor);

        layout_processor::process(geoms, la, layout_generator, detector, placements);

        return placements;
    }
};

} }

#endif // MAPNIK_LABEL_PLACEMENTS_POINT_HPP
