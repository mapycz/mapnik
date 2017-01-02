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
#ifndef MAPNIK_LABEL_PLACEMENT_SPLIT_MULTI_HPP
#define MAPNIK_LABEL_PLACEMENT_SPLIT_MULTI_HPP

#include <mapnik/util/noncopyable.hpp>
#include <mapnik/geometry_split_multi.hpp>

namespace mapnik { namespace label_placement {

// TODO: rename to apply_multi_policy?
template <typename SubLayout>
class split_multi : util::noncopyable
{
public:
    using params_type = label_placement::placement_params;

    split_multi(params_type const & params)
    : sublayout_(params),
      params_(params)
    {
    }

    template <typename LayoutGenerator, typename Detector, typename Geom>
    bool try_placement(
        LayoutGenerator & layout_generator,
        Detector & detector,
        Geom & geom)
    {
        using geom_type = geometry::cref_geometry<double>::geometry_type;
        std::list<geom_type> geoms;
        apply_multi_policy(params_.feature.get_geometry(), geoms,
            layout_generator.multi_policy());
        return sublayout_.try_placement(layout_generator, detector, geoms);
    }

private:
    SubLayout sublayout_;
    params_type const & params_;
};

} }

#endif // MAPNIK_LABEL_PLACEMENT_SPLIT_MULTI_HPP
