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
#include <mapnik/group/group_symbolizer_helper.hpp>
#include <mapnik/label_collision_detector.hpp>
#include <mapnik/geometry.hpp>
#include <mapnik/vertex_processor.hpp>
#include <mapnik/debug.hpp>
#include <mapnik/symbolizer.hpp>
#include <mapnik/value_types.hpp>
#include <mapnik/text/placements/base.hpp>
#include <mapnik/text/placements/dummy.hpp>
#include <mapnik/vertex_cache.hpp>
#include <mapnik/tolerance_iterator.hpp>

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
      scale_factor_(scale_factor)
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
    return push_to_detector(pos, angle, ignore_placement, box);
}

bool marker_layout::try_placement(
    marker_layout_generator & layout_generator,
    pixel_position const& pos)
{
    return push_to_detector(pos, 0, ignore_placement, box);

    std::list<box_element> const & box_elements = layout_generator.box_elements_;

    if (box_elements.empty()) return true;

    evaluated_text_properties const & text_props = layout_generator.get_text_props();

    // offset boxes and check collision
    std::list<box2d<double>> real_boxes;
    for (auto const& box_elem : box_elements)
    {
        box2d<double> real_box = box2d<double>(box_elem.box_);
        real_box.move(pos.x, pos.y);
        if (collision(text_props, real_box, box_elem.repeat_key_))
        {
            return false;
        }
        real_boxes.push_back(real_box);
    }

    // add boxes to collision detector
    std::list<box_element>::const_iterator elem_itr = box_elements.cbegin();
    std::list<box2d<double>>::const_iterator real_itr = real_boxes.cbegin();
    while (elem_itr != box_elements.cend() && real_itr != real_boxes.cend())
    {
        detector_.insert(*real_itr, elem_itr->repeat_key_);
        elem_itr++;
        real_itr++;
    }

    layout_generator.results_.push_back(pos);

    return true;
}

// Rotates the size_ box and translates the position.
box2d<double> perform_transform(double angle, double dx, double dy) const
{
    auto tr = params_.tr * agg::trans_affine_rotation(angle).translate(dx, dy);
    return box2d<double>(params_.size, tr);
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
    bool ignore_placement,
    box2d<double> &box)
{
    box = perform_transform(angle, pos.x, pos.y);
    if (params_.avoid_edges && !detector_.extent().contains(box))
    {
        return false;
    }
    if (!params_.allow_overlap && !detector_.has_placement(box))
    {
        return false;
    }
    if (!ignore_placement)
    {
        detector_.insert(box);
    }
    return true;
}

bool set_direction(double & angle) const
{
    switch (params_.direction)
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
