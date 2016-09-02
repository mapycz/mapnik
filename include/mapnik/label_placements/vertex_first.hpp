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

#ifndef MAPNIK_LABEL_PLACEMENTS_VERTEX_FIRST_HPP
#define MAPNIK_LABEL_PLACEMENTS_VERTEX_FIRST_HPP

#include <mapnik/geom_util.hpp>
#include <mapnik/geometry_type.hpp>
#include <mapnik/geometry_centroid.hpp>
#include <mapnik/vertex_processor.hpp>
#include <mapnik/text/point_layout.hpp>

namespace mapnik { namespace label_placement {

namespace vertex_first_detail {

template <typename Geom>
boost::optional<point_position> get_first_vertext(Geom & geom)
{
    point_position pos;

    if (agg::is_stop(geom.vertex(&pos.coords.x, &pos.coords.y)))
    {
        return boost::none;
    }

    pos.angle = 0;

    double x1, y1;

    if (agg::is_line_to(geom.vertex(&x1, &y1)))
    {
        pos.angle = std::atan2(y1 - pos.coords.y, x1 - pos.coords.x);
    }

    return pos;
}

template <typename Layout, typename LayoutGenerator, typename Detector>
struct vertex_converter_adapter
{
    vertex_converter_adapter(Layout & layout,
        LayoutGenerator & layout_generator,
        Detector & detector)
        : layout_(layout),
          layout_generator_(layout_generator),
          detector_(detector)
    {
    }

    template <typename PathT>
    void add_path(PathT & path)
    {
        boost::optional<point_position> pos = get_first_vertext(path);
        if (!pos)
        {
            status_ = false;
            return;
        }
        status_ = layout_.try_placement(layout_generator_, detector_, *pos);
    }

    bool status() const { return status_; }
    Layout & layout_;
    LayoutGenerator & layout_generator_;
    Detector & detector_;
    bool status_ = false;
};

template <typename VertexConverter, typename VertexConverterAdapter>
struct layout_adapter
{
    using this_type = layout_adapter<VertexConverter, VertexConverterAdapter>;

    layout_adapter(
        VertexConverter & converter,
        VertexConverterAdapter & adapter)
        : vertex_converter_(converter),
          vertex_converter_adapter_(adapter)
    {
    }

    template <typename VertexAdapter>
    bool operator()(VertexAdapter & vertex_adapter)
    {
        vertex_converter_.apply(vertex_adapter, vertex_converter_adapter_);
        return vertex_converter_adapter_.status();
    }

    template <typename LayoutGenerator, typename Detector, typename Geom>
    bool try_placement(
        LayoutGenerator & generator,
        Detector & detector,
        Geom const & geom)
    {

        geometry::vertex_processor<this_type> processor(*this);
        util::apply_visitor(processor, geom);
        return vertex_converter_adapter_.status();
    }

    VertexConverter & vertex_converter_;
    VertexConverterAdapter & vertex_converter_adapter_;
};

}

template <typename Layout, typename LayoutGenerator, typename Detector, typename Placements>
struct vertex_first
{
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

        value_bool clip = params.get<value_bool, keys::clip>();
        value_double simplify_tolerance = params.get<value_double, keys::simplify_tolerance>();
        value_double smooth = params.get<value_double, keys::smooth>();
        value_double extend = params.get<value_double, keys::extend>();

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

        using adapter_type = vertex_first_detail::vertex_converter_adapter<
            Layout, LayoutGenerator, Detector>;
        adapter_type adapter(layout, layout_generator, detector);
        vertex_first_detail::layout_adapter<vertex_converter_type, adapter_type> la(
            converter, adapter);

        layout_processor::process(geoms, la, layout_generator, detector, placements);

        return placements;
    }
};

} }

#endif // MAPNIK_LABEL_PLACEMENTS_VERTEX_FIRST_HPP
