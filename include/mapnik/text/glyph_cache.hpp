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
#pragma once

// mapnik
#include <mapnik/image_compositing.hpp>
#include <mapnik/util/noncopyable.hpp>
#include <mapnik/font_engine_freetype.hpp>
#include <mapnik/text/glyph_positions.hpp>
#include <mapnik/value_types.hpp>

#pragma GCC diagnostic push
#include <mapnik/warning_ignore.hpp>

#pragma GCC diagnostic push
#include <mapnik/warning_ignore_agg.hpp>
#include "agg_pixfmt_rgba.h"
#include <agg_trans_affine.h>
#pragma GCC diagnostic pop

#include <boost/container_hash/hash.hpp>

// freetype2
extern "C"
{
#include <ft2build.h>
#include FT_FREETYPE_H
#include FT_STROKER_H
}

#pragma GCC diagnostic pop

namespace mapnik
{

struct glyph_cache_key
{
    font_face const & face;
    unsigned glyph_index;
    unsigned glyph_height;
    double halo_radius;

    std::size_t hash() const
    {
        std::size_t h = 0;
        boost::hash_combine(h, &face);
        boost::hash_combine(h, glyph_index);
        boost::hash_combine(h, glyph_height);
        boost::hash_combine(h, halo_radius);
        return h;
    }

    bool operator==(glyph_cache_key const & other) const
    {
        return &face == &other.face &&
            glyph_index == other.glyph_index &&
            glyph_height == other.glyph_height &&
            std::abs(halo_radius - other.halo_radius) < 1e-3;
    }
};

}

namespace std
{
    template<> struct hash<mapnik::glyph_cache_key>
    {
        typedef mapnik::glyph_cache_key argument_type;
        typedef std::size_t result_type;
        result_type operator()(mapnik::glyph_cache_key const& k) const noexcept
        {
            return k.hash();
        }
    };
}

namespace mapnik
{

class glyph_cache
{
public:
    glyph_cache(
        freetype_engine::font_file_mapping_type & font_mapping,
        freetype_engine::font_memory_cache_type & font_cache)
        : font_library_(),
          font_manager_(font_library_, font_mapping, font_cache)
    {
    }

    using img_type = image_gray8;

    struct value_type
    {
        value_type(unsigned width, unsigned height, int x, int y)
            : img(width, height), x(x), y(y)
        {
        }

        value_type(value_type const&) = delete;
        value_type(value_type &&) = default;

        img_type img;
        int x, y;
    };

    const value_type * get(glyph_info const & glyph, double halo_radius);

private:
    std::unordered_map<glyph_cache_key, value_type> cache_;
    font_library font_library_;
    face_manager_freetype font_manager_;
    std::mutex mutex_;

    const value_type * render(glyph_cache_key const & key,
                              glyph_info const & glyph,
                              double halo_radius);

    FT_Error select_closest_size(glyph_info const& glyph, FT_Face & face) const;

    void render_halo(img_type & dst, FT_Bitmap const & src, int radius) const;
};

}
