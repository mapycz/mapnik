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

struct planner
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
            return true;
        }

        bool operator() (markers_symbolizer const & sym) const
        {
            return true;
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

    planner(Map & map) :
        background_(map, Map::copy_styles_only()),
        foreground_(map, Map::copy_styles_only())
    {
        foreground_.reset_background();
        foreground_.reset_background_image();

        bool collision_detector_used = false;

        for (auto & lyr : map.layers())
        {
            if (!collision_detector_used)
            {
                collision_detector_used = uses_collision_detector(map, lyr);
            }
            Map & current_map = collision_detector_used ? foreground_ : background_;
            current_map.add_layer(std::move(lyr));
        }
    }

    Map background_;
    Map foreground_;
};

}

#endif // MAPNIK_PLANNER_HPP
