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

#ifndef MAPNIK_LABEL_PLACEMENT_GRID_HPP
#define MAPNIK_LABEL_PLACEMENT_GRID_HPP

#include <mapnik/geom_util.hpp>
#include <mapnik/geometry_types.hpp>
#include <mapnik/geometry_split_multi.hpp>
#include <mapnik/grid_vertex_adapter.hpp>

namespace mapnik { namespace label_placement {

template <typename T, typename Points>
struct grid_placement_finder_adapter
{
    grid_placement_finder_adapter(T dx, T dy, Points & points)
        : dx_(dx), dy_(dy),
          points_(points) {}

    template <typename PathT>
    void add_path(PathT & path) const
    {
        geometry::grid_vertex_adapter<PathT, T> gpa(path, dx_, dy_);
        gpa.rewind(0);
        double label_x, label_y;
        for (unsigned cmd; (cmd = gpa.vertex(&label_x, &label_y)) != SEG_END; )
        {
            points_.emplace_back(label_x, label_y);
        }
    }

    T dx_, dy_;
    Points & points_;
};

struct grid
{
    using vertex_converter_type = vertex_converter<clip_line_tag, clip_poly_tag, transform_tag, affine_transform_tag, extend_tag, simplify_tag, smooth_tag>;

    template <typename Layout>
    static placements_list get(Layout & layout, placement_params & params)
    {
        vertex_converter_type converter(params.query_extent, params.symbolizer,
            params.view_transform, params.proj_transform, params.affine_transform,
            params.feature, params.vars, params.scale_factor);

        value_bool clip = mapnik::get<value_bool, keys::clip>(params.symbolizer, params.feature, params.vars);
        value_double simplify_tolerance = mapnik::get<value_double, keys::simplify_tolerance>(params.symbolizer, params.feature, params.vars);
        value_double smooth = mapnik::get<value_double, keys::smooth>(params.symbolizer, params.feature, params.vars);
        value_double extend = mapnik::get<value_double, keys::extend>(params.symbolizer, params.feature, params.vars);

        if (clip) converter.template set<clip_poly_tag>();
        converter.template set<transform_tag>();
        converter.template set<affine_transform_tag>();
        if (extend > 0.0) converter.template set<extend_tag>();
        if (simplify_tolerance > 0.0) converter.template set<simplify_tag>();
        if (smooth > 0.0) converter.template set<smooth_tag>();

        placements_list placements;

        using geom_type = geometry::cref_geometry<double>::geometry_type;
        std::vector<geom_type> splitted;
        geometry::split(params.feature.get_geometry(), splitted);

        for (auto const & geom_ref : splitted)
        {
            using polygon_type = geometry::cref_geometry<double>::polygon_type;
            auto const & poly = mapnik::util::get<polygon_type>(geom_ref).get();
            geometry::polygon_vertex_adapter<double> va(poly);

            params.layout_generator.reset();

            using positions_type = std::list<pixel_position>;
            positions_type points;
            evaluated_text_properties const & text_props = params.layout_generator.get_text_props();
            grid_placement_finder_adapter<double, positions_type> ga(
                text_props.grid_cell_width, text_props.grid_cell_height, points);
            converter.apply(va, ga);

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
        }

        return placements;
    }
};

} }

#endif // MAPNIK_LABEL_PLACEMENT_GRID_HPP
