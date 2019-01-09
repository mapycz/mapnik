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
#ifndef MAPNIK_MARKER_LINE_LAYOUT_HPP
#define MAPNIK_MARKER_LINE_LAYOUT_HPP

#include <mapnik/label_placements/line_layout.hpp>
#include <mapnik/marker_line_policy.hpp>
#include <mapnik/geom_util.hpp>

namespace mapnik
{

template <typename SubLayout>
class marker_line_layout : protected label_placement::line_layout<SubLayout>
{
public:
    using params_type = label_placement::placement_params;

    marker_line_layout(params_type const & params)
        : label_placement::line_layout<SubLayout>(params),
          spacing_(get_spacing()),
          position_tolerance_(spacing_ * params.get<value_double, keys::max_error>()),
          minimum_path_length_(get_minimum_path_length(spacing_))
    {
    }

    template <typename LayoutGenerator, typename Geom>
    bool try_placement(
        LayoutGenerator & layout_generator,
        Geom & geom)
    {
        // TODO: Very dubious case needed by marker_line_placement_on_points.xml.
        // Needs to be better solved conceptually or removed.
        if (geom.type() == geometry::geometry_types::Point)
        {
            pixel_position pos;
            if (label::centroid(geom, pos.x, pos.y))
            {
                return this->sublayout_.try_placement(layout_generator, pos);
            }
            return false;
        }

        double layout_width = this->sublayout_.get_length(layout_generator);
        vertex_cache path(geom);
        marker_line_policy policy(path, layout_width, spacing_,
            position_tolerance_, minimum_path_length_);
        return label_placement::line_layout<SubLayout>::try_placement(
            layout_generator, path, policy);
    }

protected:
    double get_spacing() const
    {
        double spacing = this->params_.template get<value_double, keys::spacing>() *
            this->params_.scale_factor;
        return spacing < 1 ? 100 : spacing;
    }

    double get_minimum_path_length(double spacing) const
    {
        boost::optional<double> minimum_path_length =
            this->params_.template get_optional<
                value_double, keys::minimum_path_length>();
        return minimum_path_length ?
            (this->params_.scale_factor * *minimum_path_length) :
            (spacing / 2.0);
    }

    const double spacing_;
    const double position_tolerance_;
    const double minimum_path_length_;
};

template <typename SubLayout>
class marker_line_max_angle_layout : protected marker_line_layout<SubLayout>
{
public:
    using params_type = label_placement::placement_params;

    marker_line_max_angle_layout(params_type const & params)
        : marker_line_layout<SubLayout>(params),
          max_angle_diff_((M_PI / 180.0) * params.get<value_double, keys::max_line_angle>()),
          max_angle_distance_(params.get_optional<value_double, keys::max_line_angle_distance>())
    {
    }

    template <typename LayoutGenerator, typename Geom>
    bool try_placement(
        LayoutGenerator & layout_generator,
        Geom & geom)
    {
        double layout_width = this->sublayout_.get_length(layout_generator);
        double distance = max_angle_distance_ ?
            (this->params_.scale_factor * *max_angle_distance_) :
            layout_width;
        vertex_cache path(geom);
        marker_line_max_angle_policy policy(path, layout_width, this->spacing_,
            this->position_tolerance_, this->minimum_path_length_,
            max_angle_diff_, distance);
        return label_placement::line_layout<SubLayout>::try_placement(
            layout_generator, path, policy);
    }

protected:
    const double max_angle_diff_;
    const boost::optional<double> max_angle_distance_;
};

}//ns mapnik

#endif // MAPNIK_MARKER_LINE_LAYOUT_HPP
