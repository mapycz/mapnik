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
#include <mapnik/text/line_layout.hpp>

namespace mapnik { namespace label_placement {

template <typename Layout, typename LayoutGenerator>
struct placement_finder_adapter
{
    placement_finder_adapter(Layout & layout,
        LayoutGenerator & layout_generator)
        : layout_(layout),
          layout_generator_(layout_generator)
    {
    }

    template <typename PathT>
    void add_path(PathT & path) const
    {
        status_ = layout_.try_placement(layout_generator_, path);
    }

    bool status() const { return status_; }
    Layout & layout_;
    LayoutGenerator & layout_generator_;
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

    template <typename LayoutGenerator, typename Geom>
    bool try_placement(LayoutGenerator & generator, Geom const & geom)
    {
        return util::apply_visitor(generator, geom);
    }

private:
    VC & converter_;
    Adapter const & adapter_;
};

template <typename Layout, typename Params, typename Placements>
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

    template <typename Visitor>
    struct layout_adapter
    {
        layout_adapter(Visitor & visitor)
            : visitor_(visitor)
        {
        }

        template <typename LayoutGenerator, typename Geom>
        bool try_placement(LayoutGenerator & generator, Geom const & geom)
        {
            return util::apply_visitor(visitor_, geom);
        }

        Visitor visitor_;
    };

    static Placements get(Params & params)
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

        Layout layout(params.detector, params.dims, params.scale_factor,
            params.symbolizer, params.feature, params.vars);
        //using layout_type = line_layout<Layout>;
        //layout_type line_layout(layout, params.detector, params.dims, params.scale_factor);
        Placements placements;

        using geom_type = geometry::cref_geometry<double>::geometry_type;
        std::list<geom_type> geoms;
        geometry::split(params.feature.get_geometry(), geoms);

        using layout_generator_type = typename Params::layout_generator_type;
        using adapter_type = placement_finder_adapter<Layout, layout_generator_type>;
        using visitor_type = line_placement_visitor<adapter_type, vertex_converter_type>;
        adapter_type adapter(layout, params.layout_generator);
        visitor_type visitor(converter, adapter);
        layout_adapter<visitor_type> la(visitor);

        layout_processor::process(geoms, la, params.layout_generator, placements);

        return placements;
    }
};

} }

#endif // MAPNIK_LABEL_PLACEMENT_LINE_HPP
