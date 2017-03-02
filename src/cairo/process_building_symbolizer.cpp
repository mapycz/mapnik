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

#if defined(HAVE_CAIRO)

// mapnik
#include <mapnik/make_unique.hpp>
#include <mapnik/feature.hpp>
#include <mapnik/symbolizer.hpp>
#include <mapnik/proj_transform.hpp>
#include <mapnik/cairo/cairo_renderer.hpp>
// mapnik symbolizer generics
#include <mapnik/renderer_common/process_building_symbolizer.hpp>

// stl
#include <cmath>

namespace mapnik
{

void cairo_renderer::process(
    building_symbolizer const& sym,
    mapnik::feature_impl & feature,
    proj_transform const& prj_trans,
    context_type & context)
{
    cairo_context & cntxt = context.context;
    cairo_save_restore guard(cntxt);
    composite_mode_e comp_op = get<composite_mode_e, keys::comp_op>(sym, feature, common_.vars_);
    mapnik::color fill = get<color, keys::fill>(sym, feature, common_.vars_);
    value_double opacity = get<value_double, keys::fill_opacity>(sym, feature, common_.vars_);
    value_double height = get<value_double, keys::height>(sym, feature, common_.vars_) * common_.scale_factor_;
    value_double shadow_angle = get<value_double, keys::shadow_angle>(sym, feature, common_.vars_);
    value_double shadow_length = get<value_double, keys::shadow_length>(sym, feature, common_.vars_) * common_.scale_factor_;
    value_double shadow_opacity = get<value_double, keys::shadow_opacity>(sym, feature, common_.vars_);

    cntxt.set_operator(comp_op);

    render_building_symbolizer::apply(
        feature, prj_trans, common_.t_, height, shadow_angle, shadow_length,
        [&](path_type const& faces)
        {
            vertex_adapter va(faces);
            cntxt.set_color(fill.red()  * 0.8 / 255.0, fill.green() * 0.8 / 255.0,
                               fill.blue() * 0.8 / 255.0, fill.alpha() * opacity / 255.0);
            cntxt.add_path(va);
            cntxt.fill();
        },
        [&](path_type const& frame)
        {
            vertex_adapter va(frame);
            cntxt.set_color(fill.red()  * 0.8 / 255.0, fill.green() * 0.8/255.0,
                              fill.blue() * 0.8 / 255.0, fill.alpha() * opacity / 255.0);
            cntxt.set_line_width(common_.scale_factor_);
            cntxt.add_path(va);
            cntxt.stroke();
        },
        [&](path_type const& roof)
        {
            vertex_adapter va(roof);
            cntxt.set_color(fill, opacity);
            cntxt.add_path(va);
            cntxt.fill();
        },
        [&](path_type const& shadow)
        {
            vertex_adapter va(shadow);
            agg::conv_contour<vertex_adapter> contour(va);
            contour.width(0.5 * shadow_opacity * shadow_opacity);
            cntxt.set_color(0, 0, 0, shadow_opacity);
            cntxt.add_path(contour);
            cntxt.fill();
        });
}

}

#endif // HAVE_CAIRO
