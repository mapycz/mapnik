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

#ifndef MAPNIK_LABEL_PLACEMENT_LINE_LAYOUT_HPP
#define MAPNIK_LABEL_PLACEMENT_LINE_LAYOUT_HPP

#include <mapnik/util/noncopyable.hpp>
#include <mapnik/box2d.hpp>
#include <mapnik/tolerance_iterator.hpp>
#include <mapnik/label_placements/base.hpp>

namespace mapnik { namespace label_placement {

template <typename Layout>
struct position_accessor
{
    template <typename T>
    static T & get(T & geom)
    {
        return geom;
    }
};

template <typename SubLayout>
class line_layout : util::noncopyable
{
public:
    using box_type = box2d<double>;
    using params_type = label_placement::placement_params;

    line_layout(params_type const & params);

    template <typename LayoutGenerator, typename LineLayoutPolicy>
    bool try_placement(
        LayoutGenerator & layout_generator,
        vertex_cache & path,
        LineLayoutPolicy & policy);

    SubLayout sublayout_;
    params_type const & params_;
};

template <typename SubLayout>
line_layout<SubLayout>::line_layout(params_type const & params)
    : sublayout_(params),
      params_(params)
{
}

template <typename SubLayout>
template <typename LayoutGenerator, typename LineLayoutPolicy>
bool line_layout<SubLayout>::try_placement(
    LayoutGenerator & layout_generator,
    vertex_cache & path,
    LineLayoutPolicy & policy)
{
    bool success = false;
    while (policy.next_subpath())
    {
        if (!policy.check_size())
        {
            continue;
        }

        if (!policy.align())
        {
            continue;
        }

        do
        {
            tolerance_iterator<exponential_function> tolerance_offset(
                policy.position_tolerance());
            while (tolerance_offset.next())
            {
                vertex_cache::scoped_state state(path);
                if (policy.move(tolerance_offset.get()) &&
                    sublayout_.try_placement(layout_generator,
                        position_accessor<SubLayout>::get(path)))
                {
                    success = true;
                    break;
                }
            }
        } while (policy.forward(success));
    }
    return success;
}

} }

#endif
