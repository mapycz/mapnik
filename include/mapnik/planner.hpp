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

#ifndef MAPNIK_PLANNER_HPP
#define MAPNIK_PLANNER_HPP

namespace mapnik
{

class planner
{
    struct uses_collision_detector_visitor
    {
        bool operator() (text_symbolizer const & sym)
        {
            return true;
        }

        bool operator() (shield_symbolizer const & sym)
        {
            return true;
        }

        bool operator() (point_symbolizer const & sym)
        {
            return true;
        }

        bool operator() (markers_symbolizer const & sym)
        {
            return true;
        }

        bool operator() (group_symbolizer const & sym)
        {
            return true;
        }

        bool operator() (collision_symbolizer const & sym)
        {
            return true;
        }

        template <typename Symbolizer>
        bool operator() (Symbolizer const & sym)
        {
            return false;
        }
    };

    bool uses_collision_detector(rule const & r)
    {
        const uses_collision_detector_visitor visitor;
        for (auto const & symbolizer : t.get_symbolizers())
        {
            if (util::apply_visitor(visitor, symbolizer))
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
                throw std::config_error("Style '" + style_name + "' does not exist.");
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

    planner(Map & map)
    {
    }
};

}

#endif // MAPNIK_PLANNER_HPP
