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
#ifndef MAPNIK_LABEL_PLACEMENT_POINT_LAYOUT_HPP
#define MAPNIK_LABEL_PLACEMENT_POINT_LAYOUT_HPP

#include <mapnik/util/noncopyable.hpp>

namespace mapnik { namespace label_placement {

template <typename GeometryVisitor, typename SubLayout>
class point_layout : util::noncopyable
{

public:
    using params_type = label_placement::placement_params;

    point_layout(params_type const & params)
        : sublayout_(params),
          params_(params)
    {
    }

    template <typename Detector, typename Geoms>
    bool try_placement(
        text_layout_generator & layout_generator,
        Detector & detector,
        Geoms & geoms)
    {
        std::list<pixel_position> positions;

        for (auto const & geom : geoms)
        {
            const GeometryVisitor visitor;
            if (boost::optional<geometry::point<double>> point = util::apply_visitor(visitor, geom))
            {
                geometry::point<double> & pt = *point;
                double z = 0;
                params_.proj_transform.backward(pt.x, pt.y, z);
                params_.view_transform.forward(&pt.x, &pt.y);
                positions.emplace_back(pt.x, pt.y);
            }
        }

        return sublayout_(layout_generator, detector, positions);
    }

private:
    SubLayout sublayout_;
    params_type const & params_;
};

} }

#endif // MAPNIK_LABEL_PLACEMENT_POINT_LAYOUT_HPP
