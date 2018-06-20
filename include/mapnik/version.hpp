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

#ifndef MAPNIK_VERSION_HPP
#define MAPNIK_VERSION_HPP

#include <mapnik/stringify_macro.hpp>
#include <mapnik/config.hpp>

#include <string>

#define MAPNIK_MAJOR_VERSION 3
#define MAPNIK_MINOR_VERSION 9
#define MAPNIK_PATCH_VERSION 26

#define MAPNIK_VERSION (MAPNIK_MAJOR_VERSION*100000) + (MAPNIK_MINOR_VERSION*100) + (MAPNIK_PATCH_VERSION)

#define MAPNIK_VERSION_STRING   MAPNIK_STRINGIFY(MAPNIK_MAJOR_VERSION) "." \
                                MAPNIK_STRINGIFY(MAPNIK_MINOR_VERSION) "." \
                                MAPNIK_STRINGIFY(MAPNIK_PATCH_VERSION)

namespace mapnik
{

MAPNIK_DECL unsigned version_number();
MAPNIK_DECL std::string version_string();

}

#endif // MAPNIK_VERSION_HPP
