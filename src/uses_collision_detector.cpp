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
#include <mapnik/feature_type_style.hpp>
#include <mapnik/rule.hpp>
#include <mapnik/symbolizer.hpp>

namespace mapnik
{

struct uses_collision_detector_visitor
{
    uses_collision_detector_visitor()
    {
    }

    bool operator() (text_symbolizer const & sym) const
    {
        return true;
    }

    bool operator() (shield_symbolizer const & sym) const
    {
        return true;
    }

    bool operator() (point_symbolizer const & sym) const
    {
        return !get_ignore_placement(sym) || !get_allow_overlap(sym);
    }

    bool operator() (markers_symbolizer const & sym) const
    {
        return !get_ignore_placement(sym) || !get_allow_overlap(sym);
    }

    bool operator() (group_symbolizer const & sym) const
    {
        return true;
    }

    bool operator() (collision_symbolizer const & sym) const
    {
        return true;
    }

    template <typename Symbolizer>
    bool operator() (Symbolizer const & sym) const
    {
        return false;
    }

    template <typename Symbolizer>
    bool get_ignore_placement(Symbolizer const & sym) const
    {
		return get_property_value<Symbolizer, keys::ignore_placement>(sym);
    }

    template <typename Symbolizer>
    bool get_allow_overlap(Symbolizer const & sym) const
    {
		return get_property_value<Symbolizer, keys::allow_overlap>(sym);
    }

    template <typename Symbolizer, keys key>
    bool get_property_value(Symbolizer const & sym) const
    {
        using const_iterator = symbolizer_base::cont_type::const_iterator;
        using value_type = value_bool;
        const_iterator itr = sym.properties.find(key);
        if (itr != sym.properties.end())
        {
            if (is_expression(itr->second))
            {
                return false;
            }
            return util::apply_visitor(extract_raw_value<value_type>(), itr->second);
        }
        return mapnik::symbolizer_default<value_type, key>::value();
    }
};

bool uses_collision_detector(rule const & r)
{
    const uses_collision_detector_visitor visitor;
    for (auto const & sym : r.get_symbolizers())
    {
        if (util::apply_visitor(visitor, sym))
        {
            return true;
        }
    }
    return false;
}

bool uses_collision_detector(Map const & map, layer const & lyr)
{
    for (auto const & style_name : lyr.styles())
    {
        boost::optional<feature_type_style const&> style = map.find_style(style_name);
        if (!style)
        {
            throw std::runtime_error("Style '" + style_name + "' does not exist.");
        }
        for (auto const & rule : style->get_rules())
        {
            if (uses_collision_detector(rule))
            {
                return true;
            }
        }
    }

    for (auto const sub_lyr : lyr.layers())
    {
        if (uses_collision_detector(map, sub_lyr))
        {
            return true;
        }
    }

    return false;
}

}

