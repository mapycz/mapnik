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
#ifndef MAPNIK_MARKER_LAYOUT_HPP
#define MAPNIK_MARKER_LAYOUT_HPP

//mapnik
#include <mapnik/value_types.hpp>
#include <mapnik/pixel_position.hpp>
#include <mapnik/font_engine_freetype.hpp>
#include <mapnik/marker_layout_generator.hpp>

#include <list>

namespace mapnik {

class label_collision_detector4;
class feature_impl;
class proj_transform;
class view_transform;
class vertex_cache;
using DetectorType = label_collision_detector4;

using pixel_position_list = std::list<pixel_position>;

class marker_layout : util::noncopyable
{
public:
    using box_type = box2d<double>;

    marker_layout(
        DetectorType & detector,
        box_type const& extent,
        double scale_factor,
        symbolizer_base const& sym,
        feature_impl const& feature,
        attributes const& vars);

    bool try_placement(
        marker_layout_generator & layout_generator,
        vertex_cache const & path);

    bool try_placement(
        marker_layout_generator & layout_generator,
        pixel_position const & pos);

    inline double get_length(marker_layout_generator const &) const
    {
        return 0;
    }

protected:
    bool set_direction(double & angle) const;

    bool push_to_detector(
        pixel_position const & pos,
        double angle,
        marker_layout_generator & layout_generator,
        box2d<double> & box);

    DetectorType & detector_;
    box_type const & dims_;
    const double scale_factor_;
    const value_bool ignore_placement_;
    const value_bool allow_overlap_;
    const value_bool avoid_edges_;
    direction_enum direction_;
};

} //namespace
#endif // MAPNIK_MARKER_LAYOUT_HPP
