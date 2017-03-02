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
#include <mapnik/symbolizer_enumerations.hpp>
#include <mapnik/proj_transform.hpp>
#include <mapnik/cairo/cairo_renderer.hpp>

// mapnik symbolizer generics
#include <mapnik/renderer_common/process_raster_symbolizer.hpp>

namespace mapnik
{

void cairo_renderer::process(
    raster_symbolizer const& sym,
    mapnik::feature_impl & feature,
    proj_transform const& prj_trans,
    context_type & context)
{
    cairo_context & cntxt = context.context;
    cairo_save_restore guard(cntxt);
    render_raster_symbolizer(
        sym, feature, prj_trans, common_,
        [&](image_rgba8 const & target, composite_mode_e comp_op, double opacity,
            int start_x, int start_y) {
            cntxt.set_operator(comp_op);
            cntxt.add_image(start_x, start_y, target, opacity);
        }
    );
}

}

#endif // HAVE_CAIRO
