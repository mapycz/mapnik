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
#ifndef MAPNIK_MARKER_LAYOUT_GENERATOR
#define MAPNIK_MARKER_LAYOUT_GENERATOR

//mapnik
#include <mapnik/box2d.hpp>
#include <mapnik/pixel_position.hpp>
#include <mapnik/util/noncopyable.hpp>
#include <mapnik/vertex_cache.hpp>
#include <mapnik/attribute.hpp>
#include <mapnik/svg/svg_path_attributes.hpp>
#include <mapnik/marker.hpp> // for svg_storage_type
#include <mapnik/label_placements/base.hpp>
#include <mapnik/collision_cache.hpp>

namespace mapnik
{

class feature_impl;
class text_placement_info;

struct marker_position
{
    marker_position(
        pixel_position pos,
        double angle,
        box2d<double> box)
        : pos(pos), angle(angle), box(box)
    {
    }

    pixel_position pos;
    double angle;
    box2d<double> box;
};

using marker_positions_type = std::vector<marker_position>;

struct marker_layout_generator : util::noncopyable
{
    using params_type = label_placement::placement_params;
    using detector_type = collision_detector_type;

    marker_layout_generator(
        params_type const & params,
        detector_type & detector,
        box2d<double> marker_box,
        agg::trans_affine const & marker_trans);

    inline multi_policy_enum multi_policy() const
    {
        return multi_policy_;
    }

    const box2d<double> size_;
    const agg::trans_affine tr_;
    const multi_policy_enum multi_policy_;
    marker_positions_type placements_;
    detector_type & detector_;
};

}//ns mapnik

#endif // MAPNIK_MARKER_LAYOUT_GENERATOR
