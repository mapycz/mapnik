/*****************************************************************************
 *
 * This file is part of Mapnik (c++ mapping toolkit)
 *
 * Copyright (C) 2017 Artem Pavlenko
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

#if defined(GRID_RENDERER)

// mapnik
#include <mapnik/make_unique.hpp>
#include <mapnik/feature.hpp>
#include <mapnik/grid/grid_rasterizer.hpp>
#include <mapnik/grid/grid_renderer.hpp>
#include <mapnik/grid/grid_renderer_base.hpp>
#include <mapnik/grid/grid.hpp>
#include <mapnik/transform_path_adapter.hpp>
#include <mapnik/segment.hpp>
#include <mapnik/expression_evaluator.hpp>
#include <mapnik/expression.hpp>
#include <mapnik/symbolizer.hpp>
#include <mapnik/renderer_common/process_building_symbolizer.hpp>

// stl
#include <deque>
#include <memory>

#pragma GCC diagnostic push
#include <mapnik/warning_ignore_agg.hpp>
#include "agg_rasterizer_scanline_aa.h"
#include "agg_renderer_scanline.h"
#include "agg_scanline_bin.h"
#include "agg_conv_stroke.h"
#pragma GCC diagnostic pop

namespace mapnik
{

template <typename T>
void grid_renderer<T>::process(building_symbolizer const& sym,
                               mapnik::feature_impl & feature,
                               proj_transform const& prj_trans)
{
    using pixfmt_type = typename grid_renderer_base_type::pixfmt_type;
    using color_type = typename grid_renderer_base_type::pixfmt_type::color_type;
    using renderer_type = agg::renderer_scanline_bin_solid<grid_renderer_base_type>;
    agg::scanline_bin sl;

    grid_rendering_buffer buf(pixmap_.raw_data(), common_.width_, common_.height_, common_.width_);
    pixfmt_type pixf(buf);

    grid_renderer_base_type renb(pixf);
    renderer_type ren(renb);

    ras_ptr->reset();

    value_double height = get<value_double>(sym, keys::height, feature, common_.vars_, 0.0) * common_.scale_factor_;
    value_double shadow_angle = get<value_double, keys::shadow_angle>(sym, feature, common_.vars_);
    value_double shadow_length = get<value_double, keys::shadow_length>(sym, feature, common_.vars_) * common_.scale_factor_;

    render_building_symbolizer::apply(
        feature, prj_trans, common_.t_, height, shadow_angle, shadow_length,
        [&](path_type const& faces)
        {
            vertex_adapter va(faces);
            ras_ptr->add_path(va);
            ren.color(color_type(feature.id()));
            agg::render_scanlines(*ras_ptr, sl, ren);
            ras_ptr->reset();
        },
        [&](path_type const& frame)
        {
            vertex_adapter va(frame);
            agg::conv_stroke<vertex_adapter> stroke(va);
            ras_ptr->add_path(stroke);
            ren.color(color_type(feature.id()));
            agg::render_scanlines(*ras_ptr, sl, ren);
            ras_ptr->reset();
        },
        [&](path_type const& roof)
        {
            vertex_adapter va(roof);
            ras_ptr->add_path(va);
            ren.color(color_type(feature.id()));
            agg::render_scanlines(*ras_ptr, sl, ren);
        },
        [&](path_type const& shadow)
        {
            vertex_adapter va(shadow);
            ras_ptr->add_path(va);
            ren.color(color_type(feature.id()));
            agg::render_scanlines(*ras_ptr, sl, ren);
            ras_ptr->reset();
        });

    pixmap_.add_feature(feature);
}

template void grid_renderer<grid>::process(building_symbolizer const&,
                                           mapnik::feature_impl &,
                                           proj_transform const&);

}

#endif
