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

#ifndef MAPNIK_RENDERER_COMMON_PROCESS_COLLISION_SYMBOLIZER_HPP
#define MAPNIK_RENDERER_COMMON_PROCESS_COLLISION_SYMBOLIZER_HPP

#include <mapnik/renderer_common.hpp>
#include <mapnik/text/line_layout.hpp>
#include <mapnik/marker_grid_layout.hpp>
#include <mapnik/grid_vertex_adapter.hpp>
#include <mapnik/label_placement.hpp>
#include <mapnik/label_placements/vertex_converter.hpp>
#include <mapnik/label_placements/vertex_first_layout.hpp>
#include <mapnik/label_placements/vertex_last_layout.hpp>
#include <mapnik/label_placements/vertex_layout.hpp>
#include <mapnik/label_placements/split_multi.hpp>
#include <mapnik/label_placements/text_layout_iterator.hpp>
#include <mapnik/label_placements/geom_iterator.hpp>
#include <mapnik/label_placements/point_layout.hpp>
#include <mapnik/label_placements/point_geometry_visitor.hpp>
#include <mapnik/label_placements/interior_geometry_visitor.hpp>
#include <mapnik/marker_line_policy.hpp>

namespace mapnik {

namespace collision {

struct layout_generator : util::noncopyable
{
    using params_type = label_placement::placement_params;
    using detector_type = keyed_collision_cache<label_collision_detector4>;

    layout_generator(
        params_type const & params,
        detector_type & detector)
        : multi_policy_(params.get<multi_policy_enum, keys::multipolicy>()),
          detector_(detector)
    {
    }

    inline multi_policy_enum multi_policy() const
    {
        return multi_policy_;
    }

    const multi_policy_enum multi_policy_;
    detector_type & detector_;
};

class point_layout : util::noncopyable
{
public:
    using box_type = box2d<double>;
    using params_type = label_placement::placement_params;
    using layout_generator_type = layout_generator;
    using detector_type = layout_generator_type::detector_type;

    point_layout(params_type const & params);

    bool try_placement(
        layout_generator_type & lg,
        pixel_position const& pos);

    inline bool try_placement(
        layout_generator_type & lg,
        point_position const& pos)
    {
        return try_placement(lg, pos.coords);
    }

    inline bool try_placement(
        layout_generator_type & lg,
        vertex_cache & path)
    {
        return try_placement(lg, path.current_position());
    }

    inline double get_length(layout_generator_type const &) const
    {
        return 0;
    }

protected:
    const box2d<double> size_;
    const value_bool allow_overlap_;
    const std::vector<std::string> collision_cache_insert_;
    const std::vector<std::string> collision_cache_detect_;
};

template <typename SubLayout>
class line_layout : mapnik::line_layout<SubLayout>
{
public:
    using params_type = label_placement::placement_params;

    line_layout(params_type const & params)
        : mapnik::line_layout<SubLayout>(params),
          spacing_(get_spacing())
    {
    }

    template <typename LayoutGenerator, typename Geom>
    bool try_placement(
        LayoutGenerator & layout_generator,
        Geom & geom)
    {
        vertex_cache path(geom);
        marker_line_policy policy(path, 0, spacing_, 0);
        return mapnik::line_layout<SubLayout>::try_placement(
            layout_generator, path, policy);
    }

protected:
    double get_spacing()
    {
        double spacing = this->params_.get<value_double, keys::spacing>() *
            this->params_.scale_factor;
        return spacing < 1 ? 1 : spacing;
    }

    const double spacing_;
};

}

namespace label_placement {

struct collision_symbolizer_traits
{
    using point = split_multi<
        point_layout<point_geometry_visitor,
            geom_iterator<
                collision::point_layout>>>;
    using interior = split_multi<
        point_layout<interior_geometry_visitor,
            geom_iterator<
                collision::point_layout>>>;
    using vertex = split_multi<
        geom_iterator<
            vertex_layout<
                collision::point_layout>>>;
    using grid = split_multi<
        geom_iterator<
            vertex_converter<
                marker_grid_layout<geometry::grid_vertex_adapter,
                    collision::point_layout>>>>;
    using alternating_grid = split_multi<
        geom_iterator<
            vertex_converter<
                marker_grid_layout<geometry::alternating_grid_vertex_adapter,
                    collision::point_layout>>>>;
    using line = split_multi<
        geom_iterator<
            vertex_converter<
                collision::line_layout<
                    collision::point_layout>>>>;
    using vertex_first = split_multi<
        geom_iterator<
            vertex_converter<
                vertex_first_layout<
                    collision::point_layout>>>>;
    using vertex_last = split_multi<
        geom_iterator<
            vertex_converter<
                vertex_last_layout<
                    collision::point_layout>>>>;

    using layout_generator_type = collision::layout_generator;
};

}

void process_collision_symbolizer(
    collision_symbolizer const & sym,
    mapnik::feature_impl & feature,
    proj_transform const & prj_trans,
    renderer_common & common);

} // namespace mapnik

#endif // MAPNIK_RENDERER_COMMON_PROCESS_COLLISION_SYMBOLIZER_HPP
