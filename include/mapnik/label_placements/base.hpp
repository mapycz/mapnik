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

#ifndef MAPNIK_LABEL_PLACEMENT_BASE_HPP
#define MAPNIK_LABEL_PLACEMENT_BASE_HPP

#include <mapnik/label_collision_detector.hpp>
#include <mapnik/font_engine_freetype.hpp>
#include <mapnik/symbol_cache.hpp>
#include <mapnik/symbolizer_base.hpp>

namespace mapnik { namespace label_placement { namespace detail {

template <typename DetectorT, typename FaceManagerT>
struct label_placement_params
{
    DetectorT & detector;
    FaceManagerT & font_manager;
    mapnik::proj_transform const & proj_transform;
    mapnik::view_transform const & view_transform;
    agg::trans_affine const & affine_transform;
    symbolizer_base const & symbolizer;
    feature_impl const & feature;
    attributes const & vars;
    box2d<double> dims;
    box2d<double> const & query_extent;
    double scale_factor;
    mapnik::symbol_cache const & symbol_cache;
};

}

using placement_params = detail::label_placement_params<label_collision_detector4, face_manager_freetype>;

} }

#endif // MAPNIK_LABEL_PLACEMENT_BASE_HPP
