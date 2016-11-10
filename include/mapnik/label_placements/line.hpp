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

#ifndef MAPNIK_LABEL_PLACEMENT_LINE_HPP
#define MAPNIK_LABEL_PLACEMENT_LINE_HPP

#include <mapnik/geom_util.hpp>
#include <mapnik/geometry_types.hpp>
#include <mapnik/geometry_split_multi.hpp>

namespace mapnik { namespace label_placement {

struct placement_finder_adapter
{
    placement_finder_adapter(placement_finder & finder)
        : finder_(finder) {}

    template <typename PathT>
    void add_path(PathT & path) const
    {
        status_ = finder_.find_line_placements(path, false);
    }

    bool status() const { return status_; }
    placement_finder & finder_;
    mutable bool status_ = false;
};

template <typename Adapter, typename VC>
class line_placement_visitor
{
public:
    line_placement_visitor(VC & converter,
                                 Adapter const & adapter)
        : converter_(converter), adapter_(adapter)
    {
    }

    bool operator()(geometry::line_string<double> const & geo) const
    {
        geometry::line_string_vertex_adapter<double> va(geo);
        converter_.apply(va, adapter_);
        return adapter_.status();
    }

    bool operator()(geometry::polygon<double> const & geo) const
    {
        geometry::polygon_vertex_adapter<double> va(geo);
        converter_.apply(va, adapter_);
        return adapter_.status();
    }

    template <typename T>
    bool operator()(T const&) const
    {
        return false;
    }

private:
    VC & converter_;
    Adapter const & adapter_;
};

struct line
{
    using vertex_converter_type = vertex_converter<
        clip_line_tag,
        clip_poly_tag,
        transform_tag,
        affine_transform_tag,
        extend_tag,
        simplify_tag,
        smooth_tag>;

    static placements_list get(placement_params & params)
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

        text_placement_info_ptr info_ptr = mapnik::get<text_placements_ptr>(
            params.symbolizer, keys::text_placements_)->get_placement_info(
                params.scale_factor, params.feature, params.vars, params.symbol_cache);
        placement_finder finder(params.feature, params.vars, params.detector,
            params.dims, *info_ptr, params.font_manager, params.scale_factor);

        placements_list placements;

        using geom_type = geometry::cref_geometry<double>::geometry_type;
        std::list<geom_type> geoms;
        geometry::split(params.feature.get_geometry(), geoms);

        while (!geoms.empty() && finder.next_position())
        {
            for (auto it = geoms.begin(); it != geoms.end(); )
            {
                placement_finder_adapter adapter(finder);
                line_placement_visitor<placement_finder_adapter, vertex_converter_type> v(converter, adapter);
                if (util::apply_visitor(v, *it))
                {
                    it = geoms.erase(it);
                }
                else
                {
                    ++it;
                }
            }

            if (!finder.layouts_->placements_.empty())
            {
                placements.emplace_back(std::move(finder.layouts_));
            }
        }

        return placements;
    }
};

} }

#endif // MAPNIK_LABEL_PLACEMENT_LINE_HPP
