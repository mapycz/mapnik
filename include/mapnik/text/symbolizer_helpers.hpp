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
#ifndef SYMBOLIZER_HELPERS_HPP
#define SYMBOLIZER_HELPERS_HPP

// mapnik
#include <mapnik/text/placement_finder.hpp>
#include <mapnik/text/placements/base.hpp>
#include <mapnik/vertex_converters.hpp>
#include <mapnik/symbol_cache.hpp>
#include <mapnik/geometry.hpp>
#include <mapnik/text/glyph_positions.hpp>
#include <mapnik/text/text_properties.hpp>
#include <mapnik/label_placement.hpp>
#include <mapnik/marker.hpp>
#include <mapnik/group/group_symbolizer_helper.hpp>

namespace mapnik {

class feature_impl;
class proj_transform;
class view_transform;
struct symbolizer_base;

struct text_symbolizer_traits
{
    using point = point_layout;
    using interior = point_layout;
    using vertex = point_layout;
    using grid = point_layout;
    using line = text_extend_line_layout<line_layout<single_line_layout>>;

    using placements_type = placements_list;
    using layout_generator_type = text_layout_generator;
    using params_type = label_placement::placement_params<layout_generator_type>;
};

struct shield_symbolizer_traits
{
    using point = shield_layout;
    using interior = shield_layout;
    using vertex = shield_layout;
    using grid = shield_layout;
    using line = line_layout<shield_layout>;

    using placements_type = placements_list;
    using layout_generator_type = text_layout_generator;
    using params_type = label_placement::placement_params<layout_generator_type>;
};

struct group_symbolizer_traits
{
    using point = group_point_layout;
    using interior = group_point_layout;
    using vertex = group_point_layout;
    using grid = group_point_layout;
    using line = line_layout<group_point_layout>;

    using placements_type = std::vector<pixel_position_list>;
    using layout_generator_type = group_layout_generator;
    using params_type = label_placement::placement_params<layout_generator_type>;
};

template <typename Traits>
class text_symbolizer_helper
{
public:
    using params_type = typename Traits::params_type;
    using layout_generator_type = typename Traits::layout_generator_type;

    template <typename FaceManagerT, typename DetectorT>
    static typename Traits::placements_type get(
        symbolizer_base const& sym,
        feature_impl const& feature,
        attributes const& vars,
        proj_transform const& prj_trans,
        unsigned width,
        unsigned height,
        double scale_factor,
        view_transform const& t,
        FaceManagerT & font_manager,
        DetectorT & detector,
        box2d<double> const& query_extent,
        agg::trans_affine const affine_trans,
        symbol_cache const& sc)
    {
        text_placement_info_ptr placement_info = mapnik::get<text_placements_ptr>(
            sym, keys::text_placements_)->get_placement_info(scale_factor,
                feature, vars, sc);
        layout_generator_type layout_generator(feature, vars,
            font_manager, scale_factor, *placement_info);

        const label_placement_enum placement_type =
            layout_generator.get_text_props().label_placement;

        params_type params {
            detector, font_manager, layout_generator, prj_trans, t, affine_trans, sym,
            feature, vars, box2d<double>(0, 0, width, height), query_extent,
            scale_factor, sc };

        return label_placement::finder<Traits>::get(placement_type, params);
    }
};

} //namespace mapnik

#endif // SYMBOLIZER_HELPERS_HPP
