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

#ifndef MAPNIK_CAIRO_RENDER_PATTERN_HPP
#define MAPNIK_CAIRO_RENDER_PATTERN_HPP

#include <mapnik/image.hpp>
#include <mapnik/value_types.hpp>
#include <mapnik/symbolizer_enumerations.hpp>
#include <mapnik/cairo/cairo_context.hpp>

namespace mapnik {

class symbolizer_base;
class feature_impl;
class renderer_common;
class marker_null;
class marker_svg;
class marker_rgba8;

// TODO: Share code with common_pattern_process_visitor in
// renderer_common
struct cairo_common_pattern_process_visitor
{
    using image_type = image_rgba8;

    cairo_common_pattern_process_visitor(
        renderer_common const & common,
        symbolizer_base const & sym,
        feature_impl const & feature);

    cairo_surface_ptr operator() (marker_null const &) const;

    template <typename Marker>
    cairo_surface_ptr operator() (Marker const & marker) const;

private:
    cairo_common_pattern_process_visitor(
        renderer_common const & common,
        symbolizer_base const & sym,
        feature_impl const & feature,
        boost::optional<value_double> spacing,
        boost::optional<value_double> spacing_x,
        boost::optional<value_double> spacing_y);

    agg::trans_affine transform(box2d<double> & bbox) const;

    cairo_surface_ptr render_pattern(
        marker_svg const & marker,
        box2d<double> const & bbox,
        agg::trans_affine tr) const;

    cairo_surface_ptr render_pattern(
        marker_rgba8 const& marker,
        box2d<double> const & bbox,
        agg::trans_affine tr) const;

    cairo_surface_ptr render_pattern_alternating(
        marker_svg const & marker,
        box2d<double> const & bbox,
        agg::trans_affine tr) const;

    cairo_surface_ptr render_pattern_alternating(
        marker_rgba8 const& marker,
        box2d<double> const & bbox,
        agg::trans_affine tr) const;

    renderer_common const & common_;
    symbolizer_base const & sym_;
    feature_impl const & feature_;
    const value_double opacity_;
    const value_double spacing_x_;
    const value_double spacing_y_;
    const pattern_lacing_mode_enum lacing_;
};

} // namespace mapnik

#endif // MAPNIK_CAIRO_RENDER_PATTERN_HPP
