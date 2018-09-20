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

#include <mapnik/marker_layout_generator.hpp>

namespace mapnik
{

marker_layout_generator::marker_layout_generator(
    params_type const & params,
    detector_type & detector,
    box2d<double> marker_box,
    agg::trans_affine const & marker_trans)
    : size_(marker_box, marker_trans),
      multi_policy_(params.get<multi_policy_enum, keys::multipolicy>()),
      detector_(detector)
{
}

}// ns mapnik
