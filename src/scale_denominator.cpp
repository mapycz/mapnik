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
#include <mapnik/scale_denominator.hpp>
#include <mapnik/global.hpp>

// stl
#include <cmath>

namespace mapnik {

static const double meters_per_degree = 6378137 * 2 * M_PI / 360;

double scale_denominator(double map_scale, bool geographic)
{
    // What determines the actual scale of the map when printed, is
    // determined by an additional factor which is the number of pixels that
    // make up an inch--Pixels per inch(PPI). Various digital screens have
    // different pixel sizes and therefore PPI differs in ways that software
    // often cannot be aware of. Mapnik calculates it's default at about 90.7
    // PPI, which originates from an assumed standard pixel size of 0.28
    // millimeters as defined by the OGC (Open Geospatial Consortium) SLD
    // (Styled Layer Descriptor) Specification.
    //
    // The value scale_denominator yields the map scale as given by the
    // scale() method, divided by 0.00028.
    double denom = map_scale / 0.00028;
    if (geographic) denom *= meters_per_degree;
    return denom;
}
}
