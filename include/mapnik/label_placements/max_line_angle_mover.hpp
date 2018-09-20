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

#ifndef MAPNIK_MAX_LINE_ANGLE_MOVER_HPP
#define MAPNIK_MAX_LINE_ANGLE_MOVER_HPP

#include <mapnik/vertex_cache.hpp>

namespace mapnik {

struct max_line_angle_mover
{
    max_line_angle_mover(
        vertex_cache & path,
        double max_angle_diff,
        double max_angle_distance)
        : path_(path),
          max_angle_diff_(max_angle_diff),
          max_angle_distance_(max_angle_distance)
    {
    }

    inline bool move(double distance)
    {
        vertex_cache::scoped_state state(path_);
        if (path_.move(-max_angle_distance_, max_angle_diff_))
        {
            state.restore();
            vertex_cache::scoped_state state2(path_);
            if (path_.move(max_angle_distance_, max_angle_diff_))
            {
                return true;
            }
        }

        return false;
    }

    vertex_cache & path_;
    const double max_angle_diff_;
    const double max_angle_distance_;
};

}

#endif
