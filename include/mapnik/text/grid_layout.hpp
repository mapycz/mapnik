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
#ifndef MAPNIK_TEXT_GRID_LAYOUT_HPP
#define MAPNIK_TEXT_GRID_LAYOUT_HPP

//mapnik
#include <mapnik/box2d.hpp>
#include <mapnik/pixel_position.hpp>
#include <mapnik/text/text_layout.hpp>
#include <mapnik/text/glyph_positions.hpp>
#include <mapnik/text/rotation.hpp>
#include <mapnik/util/noncopyable.hpp>
#include <mapnik/extend_converter.hpp>
#include <mapnik/vertex_cache.hpp>
#include <mapnik/text/text_layout_generator.hpp>
#include <mapnik/label_placements/base.hpp>
#include <mapnik/vertex_converters.hpp>
#include <mapnik/vertex_adapters.hpp>
#include <mapnik/grid_vertex_adapter.hpp>

namespace mapnik
{

template <typename SubLayout>
class grid_layout : util::noncopyable
{
public:
    using params_type = label_placement::placement_params;

    grid_layout(params_type const & params);

    template <typename LayoutGenerator, typename Detector, typename Geom>
    bool try_placement(
        LayoutGenerator & layout_generator,
        Detector & detector,
        Geom const & geom);

protected:
    template <typename LayoutGenerator, typename Detector, typename Geom>
    bool try_placement(
        LayoutGenerator & layout_generator,
        Detector & detector,
        Geom const & geom_ref,
        double dx, double dy);

    using vertex_converter_type = vertex_converter<
        clip_line_tag,
        clip_poly_tag,
        transform_tag,
        affine_transform_tag,
        extend_tag,
        simplify_tag,
        smooth_tag>;

    SubLayout sublayout_;
    params_type const & params_;
    vertex_converter_type converter_;
};

namespace detail {

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

}

template <typename SubLayout>
grid_layout<SubLayout>::grid_layout(params_type const & params)
    : sublayout_(params),
      params_(params),
      converter_(params.query_extent, params.symbolizer,
          params.view_transform, params.proj_transform,
          params.affine_transform, params.feature,
          params.vars, params.scale_factor)
{
    value_bool clip = mapnik::get<value_bool, keys::clip>(params.symbolizer, params.feature, params.vars);
    value_double simplify_tolerance = mapnik::get<value_double, keys::simplify_tolerance>(params.symbolizer, params.feature, params.vars);
    value_double smooth = mapnik::get<value_double, keys::smooth>(params.symbolizer, params.feature, params.vars);
    value_double extend = mapnik::get<value_double, keys::extend>(params.symbolizer, params.feature, params.vars);

    if (clip) converter_.template set<clip_poly_tag>();
    converter_.template set<transform_tag>();
    converter_.template set<affine_transform_tag>();
    if (extend > 0.0) converter_.template set<extend_tag>();
    if (simplify_tolerance > 0.0) converter_.template set<simplify_tag>();
    if (smooth > 0.0) converter_.template set<smooth_tag>();
}

template <typename SubLayout>
template <typename LayoutGenerator, typename Detector, typename Geom>
bool grid_layout<SubLayout>::try_placement(
    LayoutGenerator & layout_generator,
    Detector & detector,
    Geom const & geom)
{
    evaluated_text_properties const & text_props = layout_generator.get_text_props();
    double dx = text_props.grid_cell_width * params_.scale_factor;
    double dy = text_props.grid_cell_height * params_.scale_factor;
    return try_placement(layout_generator, detector, geom, dx, dy);
}

template <typename SubLayout>
template <typename LayoutGenerator, typename Detector, typename Geom>
bool grid_layout<SubLayout>::try_placement(
    LayoutGenerator & layout_generator,
    Detector & detector,
    Geom const & geom_ref,
    double dx, double dy)
{
    using polygon_type = geometry::cref_geometry<double>::polygon_type;
    auto const & poly = mapnik::util::get<polygon_type>(geom_ref).get();
    geometry::polygon_vertex_adapter<double> va(poly);
    using positions_type = std::list<pixel_position>;
    positions_type points;
    detail::grid_placement_finder_adapter<double, positions_type> ga(dx, dy, points);
    converter_.apply(va, ga);

    bool success = false;

    for (auto const & point : points)
    {
        success |= sublayout_.try_placement(layout_generator, detector, point);
    }

    return success;
}

}//ns mapnik

#endif // MAPNIK_TEXT_GRID_LAYOUT_HPP
