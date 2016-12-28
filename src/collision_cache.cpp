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

#include <mapnik/collision_cache.hpp>
#include <mapnik/debug.hpp>

#pragma GCC diagnostic push
#include <mapnik/warning_ignore.hpp>
#include <boost/spirit/include/qi.hpp>
#include <boost/spirit/include/phoenix.hpp>
#pragma GCC diagnostic pop

namespace mapnik
{

std::vector<std::string> parse_collision_detector_keys(
    boost::optional<std::string> const & keys)
{
    if (!keys)
    {
        return std::vector<std::string>();
    }

    std::vector<std::string> parsed_keys;
    boost::spirit::ascii::space_type space;
    std::string::const_iterator iter = keys->begin();
    std::string::const_iterator end = keys->end();

    using namespace boost::spirit::qi;
    using namespace boost::phoenix;
    char_type char_;
    as_string_type as_string;
    _1_type _1;
    _val_type _val;

    if (!phrase_parse(iter, end,
        as_string[+(char_ - ',')][push_back(_val, _1)] % ',',
        space, parsed_keys))
    {
        MAPNIK_LOG_ERROR(parse_collision_detector_keys) <<
            "Something went wrong during parsing collision cache list: '" <<
            *keys << "'. Using the whole string as the key.";
        parsed_keys.emplace_back(*keys);
    }

    return parsed_keys;
}

}
