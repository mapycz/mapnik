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

#ifndef MAPNIK_PARALLEL_BLUR
#define MAPNIK_PARALLEL_BLUR

#include <mapnik/config.hpp>
#include <mapnik/image.hpp>

namespace mapnik {

MAPNIK_DECL void stack_blur_rgba32_parallel(image_rgba8 & img,
                                            unsigned rx,
                                            unsigned ry,
                                            unsigned jobs);

} // end ns mapnik

#endif // MAPNIK_PARALLEL_BLUR
