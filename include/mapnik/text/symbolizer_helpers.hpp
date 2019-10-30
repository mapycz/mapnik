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
#include <mapnik/text/placements/base.hpp>
#include <mapnik/vertex_converters.hpp>
#include <mapnik/symbol_cache.hpp>
#include <mapnik/geometry.hpp>
#include <mapnik/text/glyph_positions.hpp>
#include <mapnik/text/text_properties.hpp>
#include <mapnik/label_placement.hpp>
#include <mapnik/marker.hpp>
#include <mapnik/text/grid_layout.hpp>
#include <mapnik/text/max_line_angle_layout.hpp>
#include <mapnik/text/line_layout.hpp>
#include <mapnik/grid_vertex_adapter.hpp>
#include <mapnik/label_placements/vertex_converter.hpp>
#include <mapnik/label_placements/vertex_first_layout.hpp>
#include <mapnik/label_placements/vertex_last_layout.hpp>
#include <mapnik/label_placements/vertex_layout.hpp>
#include <mapnik/label_placements/split_multi.hpp>
#include <mapnik/label_placements/text_layout_iterator.hpp>
#include <mapnik/label_placements/geom_iterator.hpp>
#include <mapnik/label_placements/point_layout.hpp>
#include <mapnik/label_placements/point_geometry_visitor.hpp>
#include <mapnik/label_placements/interior_geometry_visitor.hpp>

namespace mapnik {

class feature_impl;
class proj_transform;
class view_transform;
struct symbolizer_base;

namespace label_placement {

struct text_symbolizer_traits
{
    using point = split_multi<
        point_layout<point_geometry_visitor,
            text_layout_iterator<
                text::point_layout>>>;
    using interior = split_multi<
        point_layout<interior_geometry_visitor,
            text_layout_iterator<
                text::point_layout>>>;
    using vertex = split_multi<
        text_layout_iterator<
            vertex_layout<text::point_layout>>>;
    using grid = split_multi<
        text_layout_iterator<
            vertex_converter<
                set_clip_geometry_visitor,
                grid_layout<
                    geometry::grid_vertex_adapter<
                        geometry::spiral_grid_iterator>,
                    text::point_layout>>>>;
    using alternating_grid = split_multi<
        text_layout_iterator<
            vertex_converter<
                set_clip_geometry_visitor,
                grid_layout<
                    geometry::alternating_grid_vertex_adapter<
                        geometry::spiral_grid_iterator>,
                    text::point_layout>>>>;
    using line = split_multi<
        text_layout_iterator<
            vertex_converter<
                set_line_clip_geometry_visitor,
                text_extend_line_layout<
                    text_line_layout<
                        single_line_layout>>>>>;
    using line_max_angle = split_multi<
        text_layout_iterator<
            vertex_converter<
                set_line_clip_geometry_visitor,
                max_line_angle_layout<
                    single_line_layout>>>>;
    using vertex_first = split_multi<
        text_layout_iterator<
            vertex_converter<
                set_clip_geometry_visitor,
                vertex_first_layout<text::point_layout>>>>;
    using vertex_last = split_multi<
        text_layout_iterator<
            vertex_converter<
                set_clip_geometry_visitor,
                vertex_last_layout<text::point_layout>>>>;

    using placements_type = placements_list;
    using layout_generator_type = text_layout_generator;
};

struct shield_symbolizer_traits
{
    using point = split_multi<
        point_layout<point_geometry_visitor,
            text_layout_iterator<
                text::shield_layout>>>;
    using interior = split_multi<
        point_layout<interior_geometry_visitor,
            text_layout_iterator<
                text::shield_layout>>>;
    using vertex = split_multi<
        text_layout_iterator<
            vertex_layout<text::shield_layout>>>;
    using grid = split_multi<
        text_layout_iterator<
            vertex_converter<
                set_clip_geometry_visitor,
                grid_layout<
                    geometry::grid_vertex_adapter<
                        geometry::spiral_grid_iterator>,
                    text::shield_layout>>>>;
    using alternating_grid = split_multi<
        text_layout_iterator<
            vertex_converter<
                set_clip_geometry_visitor,
                grid_layout<
                    geometry::alternating_grid_vertex_adapter<
                        geometry::spiral_grid_iterator>,
                    text::shield_layout>>>>;
    using line = split_multi<
        text_layout_iterator<
            vertex_converter<
                set_line_clip_geometry_visitor,
                text_line_layout<text::shield_layout>>>>;
    using line_max_angle = split_multi<
        text_layout_iterator<
            vertex_converter<
                set_line_clip_geometry_visitor,
                max_line_angle_layout<text::shield_layout>>>>;
    using vertex_first = split_multi<
        text_layout_iterator<
            vertex_converter<
                set_clip_geometry_visitor,
                vertex_first_layout<text::shield_layout>>>>;
    using vertex_last = split_multi<
        text_layout_iterator<
            vertex_converter<
                set_clip_geometry_visitor,
                vertex_last_layout<text::shield_layout>>>>;

    using placements_type = placements_list;
    using layout_generator_type = text_layout_generator;
};

}

template <typename Traits>
class text_symbolizer_helper
{
public:
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
        symbol_cache const& sc,
        shaper_cache & sh_cache)
    {
        label_placement::placement_params params {
            prj_trans, t, affine_trans, sym, feature, vars,
            box2d<double>(0, 0, width, height), query_extent,
            scale_factor, sc };

        text_placement_info_ptr placement_info = mapnik::get<text_placements_ptr>(
            sym, keys::text_placements_)->get_placement_info(scale_factor,
                feature, vars, sc);
        layout_generator_type layout_generator(
            params, detector, font_manager, *placement_info, sh_cache);

        const label_placement_enum placement_type =
            layout_generator.get_text_props().label_placement;

        label_placement::finder<Traits>::apply(placement_type, layout_generator, params);

        typename Traits::placements_type placements(std::move(layout_generator.placements_));
        return placements;
    }
};

} //namespace mapnik

#endif // SYMBOLIZER_HELPERS_HPP
