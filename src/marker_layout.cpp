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
#include <mapnik/label_collision_detector.hpp>
#include <mapnik/geometry.hpp>
#include <mapnik/vertex_processor.hpp>
#include <mapnik/debug.hpp>
#include <mapnik/symbolizer.hpp>
#include <mapnik/value_types.hpp>
#include <mapnik/vertex_cache.hpp>
#include <mapnik/tolerance_iterator.hpp>
#include <mapnik/marker_layout.hpp>

namespace mapnik { //namespace detail {

marker_layout::marker_layout(
        DetectorType &detector,
        box2d<double> const& extent,
        double scale_factor,
        symbolizer_base const& sym,
        feature_impl const& feature,
        attributes const& vars)
    : detector_(detector),
      dims_(extent),
      scale_factor_(scale_factor),
      ignore_placement_(get<value_bool, keys::ignore_placement>(sym, feature, vars))
      allow_overlap_(get<value_bool, keys::allow_overlap>(sym, feature, vars)),
      avoid_edges_(get<value_bool, keys::avoid_edges>(sym, feature, vars)),
      direction_(get<direction_enum, keys::direction>(sym, feature, vars))
{
}

bool marker_layout::try_placement(
    marker_layout_generator & layout_generator,
    vertex_cache const & path)
{
    pixel_position const & pos = geom.current_position();
    double angle = path.angle();
    if (set_direction(angle))
    {
        return false;
    }
    return push_to_detector(pos, angle, layout_generator, box);
}

bool marker_layout::try_placement(
    marker_layout_generator & layout_generator,
    pixel_position const & pos)
{
    return push_to_detector(pos, 0, layout_generator, box);
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
    marker_layout_generator & lg,
    box2d<double> & box)
{
    auto tr = lg.trans_ * agg::trans_affine_rotation(angle).translate(pos.x, pos.y);
    box = box2d<double>(lg.size_, tr);
    if (avoid_edges_ && !detector_.extent().contains(box))
    {
        return false;
    }
    if (!allow_overlap_ && !detector_.has_placement(box))
    {
        return false;
    }
    if (!ignore_placement_)
    {
        detector_.insert(box);
    }
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
