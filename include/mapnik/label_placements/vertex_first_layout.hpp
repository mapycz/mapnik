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
#ifndef MAPNIK_LABEL_PLACEMENT_VERTEX_FIRST_LAYOUT_HPP
#define MAPNIK_LABEL_PLACEMENT_VERTEX_FIRST_LAYOUT_HPP

#include <mapnik/geometry/box2d.hpp>
#include <mapnik/util/noncopyable.hpp>
#include <mapnik/vertex_converters.hpp>
#include <mapnik/vertex_adapters.hpp>

namespace mapnik { namespace label_placement {

template <typename SubLayout>
class vertex_first_layout : util::noncopyable
{
public:
    using box_type = box2d<double>;
    using params_type = label_placement::placement_params;

    vertex_first_layout(params_type const & params)
        : sublayout_(params)
    {
    }

    template <typename LayoutGenerator, typename Path>
    bool try_placement(
        LayoutGenerator & layout_generator,
        Path & path)
    {
        boost::optional<point_position> pos = get_first_vertext(path);
        if (!pos)
        {
            return false;
        }
        return sublayout_.try_placement(layout_generator, *pos);
    }

protected:
    template <typename Geom>
    boost::optional<point_position> get_first_vertext(Geom & geom)
    {
        point_position pos;

        if (agg::is_stop(geom.vertex(&pos.coords.x, &pos.coords.y)))
        {
            return boost::none;
        }

        pos.angle = 0;

        double x1, y1;

        if (agg::is_line_to(geom.vertex(&x1, &y1)))
        {
            pos.angle = std::atan2(y1 - pos.coords.y, x1 - pos.coords.x);
        }

        return pos;
    }

    SubLayout sublayout_;
};

} }

#endif // MAPNIK_LABEL_PLACEMENT_VERTEX_FIRST_LAYOUT_HPP
