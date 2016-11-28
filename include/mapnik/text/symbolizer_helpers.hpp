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

namespace mapnik {

class feature_impl;
class proj_transform;
class view_transform;
struct symbolizer_base;

class text_symbolizer_helper
{
public:
    template <typename FaceManagerT, typename DetectorT>
    static placements_list get(symbolizer_base const& sym,
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
        box2d<double> dims(0, 0, width, height);
        text_placement_info_ptr placement_info = mapnik::get<text_placements_ptr>(
            sym, keys::text_placements_)->get_placement_info(scale_factor,
                feature, vars, sc);
        text_layout_generator layout_generator(feature, vars,
            font_manager, scale_factor, *placement_info);

        const label_placement_enum placement_type = layout_generator.get_text_props().label_placement;

        label_placement::placement_params params {
            detector, font_manager, layout_generator, prj_trans, t, affine_trans, sym,
            feature, vars, box2d<double>(0, 0, width, height), query_extent,
            scale_factor, sc };

        if (placement_type == LINE_PLACEMENT)
        {
            single_line_layout layout(params.detector, params.dims, params.scale_factor);
            return label_placement::line::get(layout, params);
        }

        point_layout layout(params.detector, params.dims, params.scale_factor);
        return label_placement::finder::get(placement_type, layout, params);
    }
};

class shield_symbolizer_helper
{
public:
    template <typename FaceManagerT, typename DetectorT>
    static placements_list get(symbolizer_base const& sym,
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
        box2d<double> dims(0, 0, width, height);
        text_placement_info_ptr placement_info = mapnik::get<text_placements_ptr>(
            sym, keys::text_placements_)->get_placement_info(scale_factor,
                feature, vars, sc);

        text_layout_generator layout_generator(feature, vars,
            font_manager, scale_factor, *placement_info);
        shield_layout layout(detector, dims, scale_factor, sym, feature, vars);

        label_placement_enum placement_type = layout_generator.get_text_props().label_placement;

        label_placement::placement_params params {
            detector, font_manager, layout_generator, prj_trans, t, affine_trans, sym,
            feature, vars, box2d<double>(0, 0, width, height), query_extent,
            scale_factor, sc };

        return label_placement::finder::get(placement_type, layout, params);
    }
};

} //namespace mapnik

#endif // SYMBOLIZER_HELPERS_HPP
