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
#ifndef MAPNIK_LABEL_PLACEMENT_GEOM_ITERATOR_HPP
#define MAPNIK_LABEL_PLACEMENT_GEOM_ITERATOR_HPP

#include <mapnik/util/noncopyable.hpp>

namespace mapnik { namespace label_placement {

template <typename SubLayout>
class geom_iterator : util::noncopyable
{
public:
    using params_type = label_placement::placement_params;

    geom_iterator(params_type const & params)
        : sublayout_(params)
    {
    }

    template <typename Detector, typename Geoms>
    bool try_placement(
        text_layout_generator & layout_generator,
        Detector & detector,
        Geoms & geoms)
    {
        bool success = false;

        for (auto & geom : geoms)
        {
            success |= sublayout_.try_placement(layout_generator, detector, geom);
        }

        return success;
    }

private:
    SubLayout sublayout_;
};

} }

#endif // MAPNIK_LABEL_PLACEMENT_GEOM_ITERATOR_HPP
