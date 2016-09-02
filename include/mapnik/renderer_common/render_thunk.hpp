/*****************************************************************************
 *
 * This file is part of Mapnik (c++ mapping toolkit)
 *
 * Copyright (C) 2016 Artem Pavlenko
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

#ifndef MAPNIK_RENDERER_COMMON_RENDER_THUNK_HPP
#define MAPNIK_RENDERER_COMMON_RENDER_THUNK_HPP

// mapnik
#include <mapnik/image_compositing.hpp> // composite_mode_e
#include <mapnik/marker.hpp> // svg_attribute_type, svg_path_ptr
#include <mapnik/symbolizer_enumerations.hpp> // halo_rasterizer_enum
#include <mapnik/svg/svg_path_attributes.hpp>
#include <mapnik/text/symbolizer_helpers.hpp>
#include <mapnik/util/noncopyable.hpp>
#include <mapnik/util/variant.hpp>

// agg
#include <agg_trans_affine.h>

namespace mapnik {

// Thunk for rendering a particular instance of a point - this
// stores all the arguments necessary to re-render this point
// symbolizer at a later time.

struct vector_marker_render_thunk : util::movable
{
    svg_path_ptr src_;
    svg_attribute_type attrs_;
    agg::trans_affine tr_;
    double opacity_;
    composite_mode_e comp_op_;
    bool snap_to_pixels_;

    vector_marker_render_thunk(svg_path_ptr const& src,
                               svg_attribute_type const& attrs,
                               agg::trans_affine const& marker_trans,
                               double opacity,
                               composite_mode_e comp_op,
                               bool snap_to_pixels)
        : src_(src), attrs_(attrs), tr_(marker_trans), opacity_(opacity),
          comp_op_(comp_op), snap_to_pixels_(snap_to_pixels)
    {}
};

struct raster_marker_render_thunk : util::movable
{
    image_rgba8 const& src_;
    agg::trans_affine tr_;
    double opacity_;
    composite_mode_e comp_op_;
    bool snap_to_pixels_;

    raster_marker_render_thunk(image_rgba8 const& src,
                               agg::trans_affine const& marker_trans,
                               double opacity,
                               composite_mode_e comp_op,
                               bool snap_to_pixels)
        : src_(src), tr_(marker_trans), opacity_(opacity), comp_op_(comp_op),
          snap_to_pixels_(snap_to_pixels)
    {}
};

struct base_text_render_thunk : util::movable
{
    placements_list placements_;
    double opacity_;
    composite_mode_e comp_op_;
    halo_rasterizer_enum halo_rasterizer_;

    base_text_render_thunk(placements_list && placements,
                      double opacity, composite_mode_e comp_op,
                      halo_rasterizer_enum halo_rasterizer)
        : placements_(std::move(placements)),
          opacity_(opacity),
          comp_op_(comp_op),
          halo_rasterizer_(halo_rasterizer)
    {}
};

// Variant type for render thunks to allow us to re-render them
// via a static visitor later.

using render_thunk = util::variant<vector_marker_render_thunk,
                                   raster_marker_render_thunk,
                                   base_text_render_thunk>;
using render_thunk_ptr = std::unique_ptr<render_thunk>;
using render_thunk_list = std::list<render_thunk_ptr>;

} // namespace mapnik

#endif // MAPNIK_RENDERER_COMMON_RENDER_THUNK_HPP
