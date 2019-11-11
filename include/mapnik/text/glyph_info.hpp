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
#ifndef MAPNIK_GLYPH_INFO_HPP
#define MAPNIK_GLYPH_INFO_HPP

//mapnik
#include <mapnik/text/evaluated_format_properties_ptr.hpp>
#include <mapnik/pixel_position.hpp>
#include <mapnik/util/noncopyable.hpp>

#include <memory>
#include <cmath>

namespace mapnik
{

class font_face;
using face_ptr = std::shared_ptr<font_face>;

struct glyph_metrics
{
    double unscaled_ymin;
    double unscaled_ymax;
    double unscaled_advance;
    double unscaled_line_height;
    double unscaled_ascender;
};

struct glyph_info : util::noncopyable
{
    glyph_info(unsigned g_index,
               unsigned c_index,
               glyph_metrics const& metrics,
               face_ptr face,
               evaluated_format_properties const& f,
               double unscaled_advance,
               double scale_multiplier,
               double offset_x, double offset_y)
        : glyph_index(g_index),
          char_index(c_index),
          format(f),
          face(face),
          offset(offset_x * scale_multiplier, offset_y * scale_multiplier),
          ymin(metrics.unscaled_ymin * 64.0 * scale_multiplier),
          ymax(metrics.unscaled_ymax * 64.0 * scale_multiplier),
          height(ymax - ymin),
          advance(unscaled_advance * scale_multiplier),
          line_height(metrics.unscaled_line_height * scale_multiplier),
          ascender(metrics.unscaled_ascender * scale_multiplier) {}

    glyph_info(glyph_info && rhs)
        : glyph_index(std::move(rhs.glyph_index)),
          char_index(std::move(rhs.char_index)),
          format(rhs.format),
          face(std::move(rhs.face)),
          offset(std::move(rhs.offset)),
          ymin(std::move(rhs.ymin)),
          ymax(std::move(rhs.ymax)),
          height(std::move(rhs.height)),
          advance(std::move(rhs.advance)),
          line_height(std::move(rhs.line_height)),
          ascender(std::move(rhs.ascender)) {}

    const unsigned glyph_index;
    // Position in the string of all characters i.e. before itemizing
    const unsigned char_index;
    evaluated_format_properties const& format;
    face_ptr const face;
    const pixel_position offset;

    const double ymin;
    const double ymax;
    const double height;
    const double advance;
    const double line_height;
    const double ascender;
};

} //ns mapnik

#endif // GLYPH_INFO_HPP
