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

#ifndef MAPNIK_PARALLELIZER_HPP
#define MAPNIK_PARALLELIZER_HPP

#include <mapnik/map.hpp>
#include <mapnik/image.hpp>

namespace mapnik { namespace parallelizer {

MAPNIK_DECL bool is_parallelizable(Map const& map);

MAPNIK_DECL void render(Map const& map,
            image_rgba8 & img,
            double scale_denom,
            double scale_factor);

} }

#endif // MAPNIK_PARALLELIZER_HPP
