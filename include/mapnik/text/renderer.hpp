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

#ifndef MAPNIK_TEXT_RENDERER_HPP
#define MAPNIK_TEXT_RENDERER_HPP

// mapnik
#include <mapnik/image_compositing.hpp>
#include <mapnik/symbolizer_enumerations.hpp>
#include <mapnik/util/noncopyable.hpp>
#include <mapnik/text/glyph_positions.hpp>
#include <mapnik/value_types.hpp>
#include <mapnik/pixel_position.hpp>

#pragma GCC diagnostic push
#include <mapnik/warning_ignore.hpp>

#pragma GCC diagnostic push
#include <mapnik/warning_ignore_agg.hpp>
#include "agg_pixfmt_rgba.h"
#include <agg_trans_affine.h>
#pragma GCC diagnostic pop
#pragma GCC diagnostic pop

namespace mapnik
{

class glyph_cache;

struct glyph_t
{
    glyph_info const& info;
    pixel_position pos;
    rotation rot;
    double size;
    box2d<double> bbox;
    glyph_t(glyph_info const& info_,
            pixel_position const& pos_,
            rotation const& rot_,
            double size_,
            box2d<double> const& bbox_)
        : info(info_),
          pos(pos_),
          rot(rot_),
          size(size_),
          bbox(bbox_) {}
};

class agg_text_renderer : private util::noncopyable
{
public:
    using pixmap_type = image_rgba8;

    agg_text_renderer(pixmap_type & pixmap,
                      composite_mode_e comp_op,
                      composite_mode_e halo_comp_op,
                      double scale_factor);

    void set_comp_op(composite_mode_e comp_op)
    {
        comp_op_ = comp_op;
    }

    void set_halo_comp_op(composite_mode_e halo_comp_op)
    {
        halo_comp_op_ = halo_comp_op;
    }

    void set_scale_factor(double scale_factor)
    {
        scale_factor_ = scale_factor;
    }

    void set_transform(agg::trans_affine const& transform);
    void set_halo_transform(agg::trans_affine const& halo_transform);

    void render(glyph_positions const& positions);

private:
    composite_mode_e comp_op_;
    composite_mode_e halo_comp_op_;
    double scale_factor_;
    agg::trans_affine transform_;
    agg::trans_affine halo_transform_;
    pixmap_type & pixmap_;
    glyph_cache & glyph_cache_;
};

}
#endif // RENDERER_HPP
