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

// mapnik
#include <mapnik/make_unique.hpp>
#include <mapnik/image_any.hpp>
#include <mapnik/feature.hpp>
#include <mapnik/agg_renderer.hpp>
#include <mapnik/agg_rasterizer.hpp>
#include <mapnik/agg_helpers.hpp>
#include <mapnik/segment.hpp>
#include <mapnik/symbolizer.hpp>
#include <mapnik/expression_evaluator.hpp>
#include <mapnik/expression.hpp>
#include <mapnik/renderer_common/process_building_symbolizer.hpp>
#include <mapnik/transform_path_adapter.hpp>

// stl
#include <deque>
#include <memory>

#pragma GCC diagnostic push
#include <mapnik/warning_ignore_agg.hpp>
#include "agg_basics.h"
#include "agg_rendering_buffer.h"
#include "agg_color_rgba.h"
#include "agg_pixfmt_rgba.h"
#include "agg_rasterizer_scanline_aa.h"
#include "agg_scanline_u.h"
#include "agg_renderer_scanline.h"
#include "agg_conv_stroke.h"
#pragma GCC diagnostic pop

namespace mapnik
{

template <typename T0,typename T1>
void agg_renderer<T0,T1>::process(building_symbolizer const& sym,
                                  mapnik::feature_impl & feature,
                                  proj_transform const& prj_trans)
{
    using ren_base = agg::renderer_base<agg::pixfmt_rgba32_pre>;
    using renderer = agg::renderer_scanline_aa_solid<ren_base>;

    buffer_type & current_buffer = buffers_.top().get();
    agg::rendering_buffer buf(current_buffer.bytes(), current_buffer.width(), current_buffer.height(), current_buffer.row_size());
    agg::pixfmt_rgba32_pre pixf(buf);
    ren_base renb(pixf);

    value_double opacity = get<value_double,keys::fill_opacity>(sym,feature, common_.vars_);
    color const& fill = get<color, keys::fill>(sym, feature, common_.vars_);
    unsigned r=fill.red();
    unsigned g=fill.green();
    unsigned b=fill.blue();
    unsigned a=fill.alpha();
    renderer ren(renb);
    agg::scanline_u8 sl;

    ras_ptr->reset();
    double gamma = get<value_double, keys::gamma>(sym, feature, common_.vars_);
    gamma_method_enum gamma_method = get<gamma_method_enum, keys::gamma_method>(sym, feature, common_.vars_);
    if (gamma != gamma_ || gamma_method != gamma_method_)
    {
        set_gamma_method(ras_ptr, gamma, gamma_method);
        gamma_method_ = gamma_method;
        gamma_ = gamma;
    }

    value_double height = get<value_double, keys::height>(sym, feature, common_.vars_) * common_.scale_factor_;
    value_double shadow_angle = get<value_double, keys::shadow_angle>(sym, feature, common_.vars_);
    value_double shadow_length = get<value_double, keys::shadow_length>(sym, feature, common_.vars_) * common_.scale_factor_;
    value_double shadow_opacity = get<value_double, keys::shadow_opacity>(sym, feature, common_.vars_);

    render_building_symbolizer::apply(
        feature, prj_trans, common_.t_, height, shadow_angle, shadow_length,
        [&,r,g,b,a,opacity](path_type const& faces)
        {
            vertex_adapter va(faces);
            ras_ptr->add_path(va);
            ren.color(agg::rgba8_pre(int(r*0.8), int(g*0.8), int(b*0.8), int(a * opacity)));
            agg::render_scanlines(*ras_ptr, sl, ren);
            this->ras_ptr->reset();
        },
        [&,r,g,b,a,opacity](path_type const& frame)
        {
            vertex_adapter va(frame);
            agg::conv_stroke<vertex_adapter> stroke(va);
            stroke.width(common_.scale_factor_);
            ras_ptr->add_path(stroke);
            ren.color(agg::rgba8_pre(int(r*0.8), int(g*0.8), int(b*0.8), int(a * opacity)));
            agg::render_scanlines(*ras_ptr, sl, ren);
            ras_ptr->reset();
        },
        [&,r,g,b,a,opacity](path_type const& roof)
        {
            vertex_adapter va(roof);
            ras_ptr->add_path(va);
            ren.color(agg::rgba8_pre(r, g, b, int(a * opacity)));
            agg::render_scanlines(*ras_ptr, sl, ren);
        },
        [&,r,g,b,a,opacity,shadow_opacity](path_type const& shadow)
        {
            vertex_adapter va(shadow);
            agg::conv_contour<vertex_adapter> contour(va);
            contour.width(0.5 * shadow_opacity * shadow_opacity);
            ras_ptr->add_path(contour);
            ren.color(agg::rgba8_pre(0, 0, 0,
                static_cast<int>(255.0 * shadow_opacity)));
            agg::render_scanlines(*ras_ptr, sl, ren);
            this->ras_ptr->reset();
        });
}

template void agg_renderer<image_rgba8>::process(building_symbolizer const&,
                                              mapnik::feature_impl &,
                                              proj_transform const&);
}
