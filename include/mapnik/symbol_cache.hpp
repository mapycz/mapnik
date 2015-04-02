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

#ifndef MAPNIK_SYMBOL_CACHE_HPP
#define MAPNIK_SYMBOL_CACHE_HPP

// stl
#include <map>
#include <vector>
#include <string>

// mapnik
#include <mapnik/box2d.hpp>
#include <mapnik/util/noncopyable.hpp>

namespace mapnik
{

class symbol_cache : util::noncopyable
{
public:
    using box_type = box2d<double>;

    struct item
    {
        item(box_type const & box) : box(box) { }
        box_type box;
    };

    void insert(std::string const & key, box_type const & value);

private:
    using list_type = std::vector<item>;
    std::map<std::string, list_type> boxes;
};

}

#endif
