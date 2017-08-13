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

#ifndef NAME_LIST_GRAMMAR_HPP
#define NAME_LIST_GRAMMAR_HPP

#pragma GCC diagnostic push
#include <mapnik/warning_ignore.hpp>
#include <boost/spirit/include/qi.hpp>
#include <boost/spirit/include/phoenix.hpp>
#pragma GCC diagnostic pop

namespace visual_tests
{

namespace qi = boost::spirit::qi;
namespace ascii = boost::spirit::ascii;

template <typename Iterator>
struct name_list_grammar : qi::grammar<Iterator, std::vector<std::string>(), ascii::space_type>
{
    name_list_grammar() : name_list_grammar::base_type(start)
    {
        using namespace boost::spirit::qi;
        using namespace boost::phoenix;

        alnum_type alnum;
        _1_type _1;
        _val_type val;
        as_string_type as_string;

        start = as_string[(+alnum)][push_back(val, _1)] % ',';
    }

    qi::rule<Iterator, std::vector<std::string>(), ascii::space_type> start;
};

}

#endif
