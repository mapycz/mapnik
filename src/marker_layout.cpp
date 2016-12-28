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

// mapnik
#include <mapnik/collision_cache.hpp>
#include <mapnik/geometry.hpp>
#include <mapnik/vertex_processor.hpp>
#include <mapnik/debug.hpp>
#include <mapnik/symbolizer.hpp>
#include <mapnik/value_types.hpp>
#include <mapnik/vertex_cache.hpp>
#include <mapnik/tolerance_iterator.hpp>
#include <mapnik/marker_layout.hpp>
#include <mapnik/util/math.hpp>

namespace mapnik {

using detector_type = keyed_collision_cache<label_collision_detector4>;

marker_layout::marker_layout(params_type const & params)
    : params_(params),
      ignore_placement_(params.get<value_bool, keys::ignore_placement>()),
      allow_overlap_(params.get<value_bool, keys::allow_overlap>()),
      avoid_edges_(params.get<value_bool, keys::avoid_edges>()),
      direction_(params.get<direction_enum, keys::direction>()),
      margin_(params.get<value_double, keys::margin>() * params.scale_factor),
      collision_cache_(params.get_optional<std::string, keys::collision_cache>())
{
}

template <typename Detector>
bool marker_layout::try_placement(
    marker_layout_generator & layout_generator,
    Detector & detector,
    vertex_cache & path)
{
    pixel_position const & pos = path.current_position();
    double angle = path.angle();
    return push_to_detector(detector, pos, angle, layout_generator);
}

template bool marker_layout::try_placement(
    marker_layout_generator & layout_generator,
    detector_type & detector,
    vertex_cache & path);

template <typename Detector>
bool marker_layout::try_placement(
    marker_layout_generator & layout_generator,
    Detector & detector,
    pixel_position const & pos)
{
    return push_to_detector(detector, pos, 0, layout_generator);
}

template bool marker_layout::try_placement(
    marker_layout_generator & layout_generator,
    detector_type & detector,
    pixel_position const & pos);

template <typename Detector>
bool marker_layout::try_placement(
    marker_layout_generator & layout_generator,
    Detector & detector,
    point_position const & pos)
{
    return push_to_detector(detector, pos.coords, pos.angle, layout_generator);
}

template bool marker_layout::try_placement(
    marker_layout_generator & layout_generator,
    detector_type & detector,
    point_position const & pos);

// Checks transformed box placement with collision detector.
// returns false if the box:
//  - a) isn't wholly inside extent and avoid_edges == true
//  - b) collides with something and allow_overlap == false
// otherwise returns true, and if ignore_placement == false,
//  also adds the box to collision detector
template <typename Detector>
bool marker_layout::push_to_detector(
    Detector & detector,
    pixel_position const & pos,
    double angle,
    marker_layout_generator & lg)
{
    if (!set_direction(angle))
    {
        return false;
    }
    auto tr = lg.tr_ * agg::trans_affine_rotation(angle).translate(pos.x, pos.y);
    box2d<double> box(lg.size_, tr);
    if (avoid_edges_ && !detector.extent().contains(box))
    {
        return false;
    }
    if (!allow_overlap_ && !detector.has_placement(box, margin_, collision_cache_))
    {
        return false;
    }
    if (!ignore_placement_)
    {
        detector.insert(box, collision_cache_);
    }
    lg.placements_.emplace_back(pos, angle, box);
    return true;
}

template bool marker_layout::push_to_detector(
    detector_type & detector,
    pixel_position const & pos,
    double angle,
    marker_layout_generator & lg);

bool marker_layout::set_direction(double & angle) const
{
    switch (direction_)
    {
        case DIRECTION_UP:
            angle = 0;
            return true;
        case DIRECTION_DOWN:
            angle = M_PI;
            return true;
        case DIRECTION_AUTO:
            if (std::fabs(util::normalize_angle(angle)) > 0.5 * M_PI)
                angle += M_PI;
            return true;
        case DIRECTION_AUTO_DOWN:
            if (std::fabs(util::normalize_angle(angle)) < 0.5 * M_PI)
                angle += M_PI;
            return true;
        case DIRECTION_LEFT:
            angle += M_PI;
            return true;
        case DIRECTION_LEFT_ONLY:
            angle += M_PI;
            return std::fabs(util::normalize_angle(angle)) < 0.5 * M_PI;
        case DIRECTION_RIGHT_ONLY:
            return std::fabs(util::normalize_angle(angle)) < 0.5 * M_PI;
        case DIRECTION_RIGHT:
        default:
            return true;
    }
}

} //namespace
