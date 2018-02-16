/*****************************************************************************
 *
 * This file is part of Mapnik (c++ mapping toolkit)
 *
 * Copyright (C) 2018 Artem Pavlenko
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

#include <mapnik/uses_collision_detector.hpp>
#include <mapnik/map.hpp>
#include <mapnik/layer.hpp>

namespace mapnik
{

void set_layer_buffer_size(Map & map)
{
    if (!map.buffer_size_collisions())
    {
        return;
    }

    for (auto & layer : map.layers())
    {
        if (!layer.buffer_size() && uses_collision_detector(map, layer))
        {
            layer.set_buffer_size(*map.buffer_size_collisions());
        }
    }
}

}
