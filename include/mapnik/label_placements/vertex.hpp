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

#ifndef MAPNIK_LABEL_PLACEMENT_VERTEX_HPP
#define MAPNIK_LABEL_PLACEMENT_VERTEX_HPP

#include <mapnik/geom_util.hpp>
#include <mapnik/geometry_types.hpp>
#include <mapnik/geometry_split_multi.hpp>

namespace mapnik {

template <typename Points>
struct apply_vertex_placement
{
    apply_vertex_placement(Points & points, view_transform const& tr, proj_transform const& prj_trans)
        : points_(points),
          tr_(tr),
          prj_trans_(prj_trans) {}

    template <typename Adapter>
    void operator() (Adapter const& va) const
    {
        double label_x, label_y, z = 0;
        va.rewind(0);
        for (unsigned cmd; (cmd = va.vertex(&label_x, &label_y)) != SEG_END; )
        {
            if (cmd != SEG_CLOSE)
            {
                prj_trans_.backward(label_x, label_y, z);
                tr_.forward(&label_x, &label_y);
                points_.emplace_back(label_x, label_y);
            }
        }
    }
    Points & points_;
    view_transform const& tr_;
    proj_transform const& prj_trans_;
};

template <typename Geom>
class label_interior_placement
{
public:
    static placements_list get(label_placement_params & params)
    {
        using positions_type = std::list<pixel_position>;
        using apply_vertex_placement = detail::apply_vertex_placement<positions_type>;
        positions_type points;
        apply_vertex_placement apply(points, params.view_transform, params.proj_transform);
        util::apply_visitor(geometry::vertex_processor<apply_vertex_placement>(apply), params.geometry);

        placement_finder finder(params.feature, params.vars, params.detector,
            params.dims, params.placement_info, params.font_manager, params.scale_factor);

        while (finder.next_position())
        {
            for (auto it = points.begin(); it != point.end(); )
            {
                if (finder.find_point_placement(*it))
                {
                    it = points.erase(it);
                }
                else
                {
                    it++;
                }
            }
        }

        return finder.placements();
    }
};

}

#endif // MAPNIK_LABEL_PLACEMENT_VERTEX_HPP
