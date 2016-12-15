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
#ifndef MAPNIK_MARKER_LINE_POLICY_HPP
#define MAPNIK_MARKER_LINE_POLICY_HPP

#include <mapnik/vertex_cache.hpp>

namespace mapnik
{

struct marker_line_policy
{
    using params_type = label_placement::placement_params;

    marker_line_policy(
        vertex_cache & path,
        marker_layout_generator const & lg,
        double layout_width,
        double spacing,
        double position_tolerance,
        params_type const & params)
        : path_(path),
          params_(params),
          layout_width_(layout_width),
          spacing_(spacing),
          position_tolerance_(position_tolerance)
    {
    }

    inline bool check_size() const
    {
        return true;
    }

    inline double get_spacing() const
    {
        return spacing_;
    }

    inline bool align()
    {
        return path_.forward(get_spacing() / 2.0);
    }

    inline bool move(double distance)
    {
        return path_.move(distance) &&
            (path_.linear_position() + layout_width_ / 2.0) < path_.length();
    }

    double position_tolerance() const
    {
        return position_tolerance_;
    }

    vertex_cache & path_;
    params_type const & params_;
    const double layout_width_;
    const double spacing_;
    const double position_tolerance_;
};


}//ns mapnik

#endif // MAPNIK_MARKER_LINE_POLICY_HPP
