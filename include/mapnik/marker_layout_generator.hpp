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
#ifndef MAPNIK_MARKER_LAYOUT_GENERATOR
#define MAPNIK_MARKER_LAYOUT_GENERATOR

//mapnik
#include <mapnik/box2d.hpp>
#include <mapnik/pixel_position.hpp>
#include <mapnik/util/noncopyable.hpp>
#include <mapnik/vertex_cache.hpp>
#include <mapnik/attribute.hpp>
#include <mapnik/svg/svg_path_attributes.hpp>
#include <mapnik/marker.hpp> // for svg_storage_type

namespace mapnik
{

class feature_impl;
class text_placement_info;

struct marker_position
{
    pixel_position pos;
};

using marker_positions_type = std::vector<marker_position>;

struct marker_layout_generator : util::noncopyable
{
    marker_layout_generator(
        feature_impl const& feature,
        attributes const& vars,
        double scale_factor,
        svg_path_ptr const & src,
        svg_path_adapter & path,
        svg_attribute_type const & attrs,
        agg::trans_affine const & marker_trans);

    bool next();
    void reset();

    inline bool has_placements() const
    {
        return !placements_.empty();
    }

    inline marker_positions_type & get_placements()
    {
        return placements_;
    }

    inline bool largest_box_only() const
    {
        // TODO
        return true;
    }

    agg::trans_affine recenter(svg_path_ptr const& src) const
    {
        coord2d center = src->bounding_box().center();
        return agg::trans_affine_translation(-center.x, -center.y);
    }

    feature_impl const& feature_;
    attributes const& vars_;
    const double scale_factor_;
    bool state_;
    const box2d<double> size_;
    agg::trans_affine tr_;
    marker_positions_type placements_;
};

}//ns mapnik

#endif // MAPNIK_MARKER_LAYOUT_GENERATOR
