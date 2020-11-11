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

marker_layout::marker_layout(params_type const & params)
    : params_(params),
      ignore_placement_(params.get<value_bool, keys::ignore_placement>()),
      allow_overlap_(params.get<value_bool, keys::allow_overlap>()),
      avoid_edges_(params.get<value_bool, keys::avoid_edges>()),
      direction_(params.get<direction_enum, keys::direction>()),
      margin_(params.get<value_double, keys::margin>() * params.scale_factor),
      repeat_distance_(params.get<value_double, keys::repeat_distance>() * params.scale_factor),
      repeat_key_(params.get<std::string, keys::file>().c_str()),
      collision_cache_insert_(parse_collision_detector_keys(
          params.get_optional<std::string, keys::collision_cache_insert>())),
      collision_cache_detect_(parse_collision_detector_keys(
          params.get_optional<std::string, keys::collision_cache_detect>()))
{
}

bool marker_layout::try_placement(
    marker_layout_generator & layout_generator,
    vertex_cache & path)
{
    pixel_position const & pos = path.current_position();
    double angle = path.angle();
    return push_to_detector(pos, angle, layout_generator);
}

bool marker_layout::try_placement(
    marker_layout_generator & layout_generator,
    pixel_position const & pos)
{
    return push_to_detector(pos, 0, layout_generator);
}

bool marker_layout::try_placement(
    marker_layout_generator & layout_generator,
    point_position const & pos)
{
    return push_to_detector(pos.coords, pos.angle, layout_generator);
}

// Checks transformed box placement with collision detector.
// returns false if the box:
//  - a) isn't wholly inside extent and avoid_edges == true
//  - b) collides with something and allow_overlap == false
// otherwise returns true, and if ignore_placement == false,
//  also adds the box to collision detector
bool marker_layout::push_to_detector(
    pixel_position const & pos,
    double angle,
    marker_layout_generator & lg)
{
    if (!set_direction(angle))
    {
        return false;
    }
    auto tr = agg::trans_affine_rotation(angle).translate(pos.x, pos.y);
    box2d<double> box(lg.size_, tr);
    detector_type & detector = lg.detector_;
    if (avoid_edges_ && !detector.extent().contains(box))
    {
        return false;
    }
    if (!allow_overlap_ && (
        (repeat_key_.length() == 0 && !detector.has_placement(
            box, margin_, collision_cache_detect_))
        ||
        (repeat_key_.length() > 0 && !detector.has_placement(
            box, margin_, repeat_key_, repeat_distance_, collision_cache_detect_))
        ))
    {
        return false;
    }
    if (!ignore_placement_)
    {
        detector.insert(box, repeat_key_, collision_cache_insert_);
    }
    lg.placements_.emplace_back(pos, angle, box);
    return true;
}

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
