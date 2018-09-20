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
#ifndef MAX_LINE_ANGLE_LAYOUT_HPP
#define MAX_LINE_ANGLE_LAYOUT_HPP

#include <mapnik/box2d.hpp>
#include <mapnik/pixel_position.hpp>
#include <mapnik/text/text_layout.hpp>
#include <mapnik/text/glyph_positions.hpp>
#include <mapnik/text/rotation.hpp>
#include <mapnik/util/noncopyable.hpp>
#include <mapnik/extend_converter.hpp>
#include <mapnik/vertex_cache.hpp>
#include <mapnik/text/point_layout.hpp>
#include <mapnik/tolerance_iterator.hpp>
#include <mapnik/text/text_line_policy.hpp>
#include <mapnik/vertex_converters.hpp>
#include <mapnik/vertex_adapters.hpp>
#include <mapnik/label_placements/line_layout.hpp>

namespace mapnik
{

template <typename SubLayout>
class max_line_angle_layout : protected label_placement::line_layout<SubLayout>
{
public:
    using box_type = box2d<double>;
    using params_type = label_placement::placement_params;

    max_line_angle_layout(params_type const & params);

    template <typename LayoutGenerator, typename Geom>
    bool try_placement(
        LayoutGenerator & layout_generator,
        Geom & geom);

private:
    const double max_angle_diff_;
    const boost::optional<double> max_angle_distance_;
};

template <typename SubLayout>
max_line_angle_layout<SubLayout>::max_line_angle_layout(
    params_type const & params)
    : label_placement::line_layout<SubLayout>(params),
      max_angle_diff_((M_PI / 180.0) * params.get<value_double, keys::max_line_angle>()),
      max_angle_distance_(params.get_optional<value_double, keys::max_line_angle_distance>())

{
}

template <typename SubLayout>
template <typename LayoutGenerator, typename Geom>
bool max_line_angle_layout<SubLayout>::try_placement(
    LayoutGenerator & layout_generator,
    Geom & geom)
{
    vertex_cache path(geom);
    double layout_width = this->sublayout_.get_length(layout_generator);
    double distance = max_angle_distance_ ?
        (this->params_.scale_factor * *max_angle_distance_) :
        layout_width;
    text_max_line_angle_policy<LayoutGenerator> policy(path,
        layout_generator, layout_width, this->params_, max_angle_diff_,
        distance);
    return label_placement::line_layout<SubLayout>::try_placement(
        layout_generator, path, policy);
}

}//ns mapnik

#endif
