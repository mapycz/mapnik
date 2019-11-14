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

#include <mapnik/renderer_common.hpp>
#include <mapnik/map.hpp>
#include <mapnik/request.hpp>
#include <mapnik/attribute.hpp>
#include <mapnik/safe_cast.hpp>

namespace mapnik {

renderer_common::renderer_common(Map const& map, unsigned width, unsigned height, double scale_factor,
                                 attributes const& vars,
                                 view_transform && t,
                                 detector_ptr detector)
   : width_(width),
     height_(height),
     scale_factor_(scale_factor),
     vars_(vars),
     query_extent_(),
     t_(t),
     detector_(detector)
{}

renderer_common::renderer_common(Map const &m, attributes const& vars, unsigned offset_x, unsigned offset_y,
                                 unsigned width, unsigned height, double scale_factor)
   : renderer_common(m, width, height, scale_factor,
                     vars,
                     view_transform(m.width(),m.height(),m.get_current_extent(),offset_x,offset_y),
                     std::make_shared<detector_type>(
                        box2d<double>(-m.buffer_size(), -m.buffer_size(),
                                      m.width() + m.buffer_size() ,m.height() + m.buffer_size())))
{}

renderer_common::renderer_common(Map const &m, attributes const& vars, unsigned offset_x, unsigned offset_y,
                                 unsigned width, unsigned height, double scale_factor,
                                 detector_ptr detector)
   : renderer_common(m, width, height, scale_factor,
                     vars,
                     view_transform(m.width(),m.height(),m.get_current_extent(),offset_x,offset_y),
                     detector)
{}

renderer_common::renderer_common(Map const &m, request const &req, attributes const& vars, unsigned offset_x, unsigned offset_y,
                                 unsigned width, unsigned height, double scale_factor)
   : renderer_common(m, width, height, scale_factor,
                     vars,
                     view_transform(req.width(),req.height(),req.extent(),offset_x,offset_y),
                     std::make_shared<detector_type>(
                        box2d<double>(-req.buffer_size(), -req.buffer_size(),
                                      req.width() + req.buffer_size() ,req.height() + req.buffer_size())))
{}

renderer_common::~renderer_common()
{
    // defined in .cpp to make this destructible elsewhere without
    // having to #include <mapnik/label_collision_detector.hpp>
}

}
