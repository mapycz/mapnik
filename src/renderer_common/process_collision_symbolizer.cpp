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

#include <mapnik/renderer_common/process_collision_symbolizer.hpp>

namespace mapnik {

namespace collision {

point_layout::point_layout(params_type const & params)
    : size_(0, 0,
          params.get<value_double, keys::width>() * params.scale_factor,
          params.get<value_double, keys::height>() * params.scale_factor),
      allow_overlap_(params.get<value_bool, keys::allow_overlap>()),
      collision_cache_insert_(parse_collision_detector_keys(
          params.get_optional<std::string, keys::collision_cache_insert>())),
      collision_cache_detect_(parse_collision_detector_keys(
          params.get_optional<std::string, keys::collision_cache_detect>()))
{
}

bool point_layout::try_placement(
    layout_generator & lg,
    pixel_position const & pos)
{
    box2d<double> box(size_);
    box.re_center(pos.x, pos.y);

    if (!allow_overlap_ && !lg.detector_.has_placement(box, collision_cache_detect_))
    {
        return false;
    }

    lg.detector_.insert(box, collision_cache_insert_);

    return true;
}

}

void process_collision_symbolizer(
    collision_symbolizer const & sym,
    mapnik::feature_impl & feature,
    proj_transform const & prj_trans,
    renderer_common & common)
{
    agg::trans_affine tr;
    label_placement::placement_params params {
        prj_trans, common.t_, tr, sym, feature, common.vars_,
        box2d<double>(0, 0, common.width_, common.height_), common.query_extent_,
        common.scale_factor_, common.symbol_cache_ };

    using traits = label_placement::collision_symbolizer_traits;
    traits::layout_generator_type layout_generator(params, *common.detector_);
    const label_placement_enum placement_type =
        params.get<label_placement_enum, keys::label_placement>();

    label_placement::finder<traits>::apply(placement_type, layout_generator, params);
}

} // namespace mapnik
