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

#ifndef MAPNIK_RENDER_PATTERN_HPP
#define MAPNIK_RENDER_PATTERN_HPP

#include <mapnik/image.hpp>
#include <mapnik/marker.hpp>
#include <mapnik/symbolizer.hpp>
#include <mapnik/renderer_common.hpp>

// fwd decl
namespace agg {
struct trans_affine;
}

namespace mapnik {

// fwd decl
struct rasterizer;
struct feature_impl;

image_rgba8 render_pattern(rasterizer & ras,
                    marker_svg const& marker,
                    box2d<double> const & bbox,
                    agg::trans_affine const& tr);

image_rgba8 render_pattern(rasterizer & ras,
                    marker_rgba8 const& marker,
                    box2d<double> const & bbox,
                    agg::trans_affine const& tr);

template <typename Symbolizer, typename Rasterizer>
struct common_pattern_process_visitor
{
    using image_type = image_rgba8;

    common_pattern_process_visitor(Rasterizer & ras,
                                   renderer_common const & common,
                                   Symbolizer const & sym,
                                   feature_impl const & feature)
        : ras_(ras),
          common_(common),
          sym_(sym),
          feature_(feature)
    {
    }

    image_type operator() (marker_null const &) const
    {
        throw std::runtime_error("This should not have been reached.");
    }

    template <typename Marker>
    image_type operator() (Marker const & marker) const
    {
        box2d<double> bbox(marker.bounding_box());
        agg::trans_affine tr(transform(bbox));
        return render_pattern(ras_, marker, bbox, tr);
    }

private:
    agg::trans_affine transform(box2d<double> & bbox) const
    {
        agg::trans_affine tr = agg::trans_affine_scaling(common_.scale_factor_);
        auto image_transform = get_optional<transform_type>(sym_, keys::image_transform);
        if (image_transform) evaluate_transform(tr, feature_, common_.vars_, *image_transform, common_.scale_factor_);
        bbox *= tr;
        coord<double, 2> c = bbox.center();
        agg::trans_affine mtx = agg::trans_affine_translation(
            0.5 * bbox.width() - c.x,
            0.5 * bbox.height() - c.y);
        return tr * mtx;
    }

    Rasterizer & ras_;
    renderer_common const & common_;
    Symbolizer const & sym_;
    feature_impl const & feature_;
};

} // namespace mapnik

#endif // MAPNIK_RENDER_PATTERN_HPP
