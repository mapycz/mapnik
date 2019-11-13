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
#include <mapnik/font_engine_freetype.hpp>
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

    std::size_t hash() const
    {
        std::size_t h = 0;
        boost::hash_combine(h, &face);
        boost::hash_combine(h, glyph_index);
        boost::hash_combine(h, glyph_height);
        return h;
    }

    bool operator==(glyph_cache_key const & other) const
    {
        return &face == &other.face &&
            glyph_index == other.glyph_index &&
            glyph_height == other.glyph_height;
    }
};

struct glyph_halo_cache_key : glyph_cache_key
{
    double halo_radius;

    std::size_t hash() const
    {
        std::size_t h = glyph_cache_key::hash();
        boost::hash_combine(h, halo_radius);
        return h;
    }

    bool operator==(glyph_halo_cache_key const & other) const
    {
        return glyph_cache_key::operator==(other) &&
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

    template<> struct hash<mapnik::glyph_halo_cache_key>
    {
        typedef mapnik::glyph_halo_cache_key argument_type;
        typedef std::size_t result_type;
        result_type operator()(mapnik::glyph_halo_cache_key const& k) const noexcept
        {
            return k.hash();
        }
    };
}

namespace mapnik
{

struct rasterizer;

struct glyph_t
{
    glyph_info const& info;
    FT_Glyph image;
    pixel_position pos;
    rotation rot;
    double size;
    box2d<double> bbox;
    glyph_t(glyph_info const& info_,
            FT_Glyph image_,
            pixel_position const& pos_,
            rotation const& rot_,
            double size_,
            box2d<double> const& bbox_)
        : info(info_),
          image(image_),
          pos(pos_),
          rot(rot_),
          size(size_),
          bbox(bbox_) {}
};

class text_renderer : private util::noncopyable
{
public:
    text_renderer (halo_rasterizer_e rasterizer,
                   composite_mode_e comp_op,
                   composite_mode_e halo_comp_op,
                   double scale_factor,
                   stroker_ptr stroker);

    void set_comp_op(composite_mode_e comp_op)
    {
        comp_op_ = comp_op;
    }

    void set_halo_comp_op(composite_mode_e halo_comp_op)
    {
        halo_comp_op_ = halo_comp_op;
    }

    void set_halo_rasterizer(halo_rasterizer_e rasterizer)
    {
        rasterizer_ = rasterizer;
    }

    void set_scale_factor(double scale_factor)
    {
        scale_factor_ = scale_factor;
    }

    /*
    void set_stroker(stroker_ptr stroker)
    {
        stroker_ = stroker;
    }
    */

    void set_transform(agg::trans_affine const& transform);
    void set_halo_transform(agg::trans_affine const& halo_transform);

protected:
    using glyph_vector = std::vector<glyph_t>;
    void prepare_glyphs(glyph_positions const& positions, bool is_mono);
    FT_Error select_closest_size(glyph_info const& glyph, FT_Face & face) const;
    halo_rasterizer_e rasterizer_;

    composite_mode_e comp_op_;
    composite_mode_e halo_comp_op_;
    double scale_factor_;
    glyph_vector glyphs_;
    stroker_ptr stroker_;
    agg::trans_affine transform_;
    agg::trans_affine halo_transform_;
};

class halo_cache
{
public:
    using key_type = std::tuple<std::string, // family
                                std::string, // face
                                unsigned, // glyph index
                                unsigned, // glyph height
                                int>; // halo radius
    using img_type = image_gray8;
    using value_type = std::unique_ptr<img_type>;

    using pixfmt_type = agg::pixfmt_rgba32_pre;

    image_gray8 const& get(glyph_info const & glyph,
                           pixfmt_type const& bitmap,
                           double halo_radius);

private:
    std::map<key_type, value_type> cache_;

    void render_halo_img(pixfmt_type const& glyph_bitmap,
                         img_type & halo_bitmap,
                         int radius);
};

class glyph_cache
{
public:
    glyph_cache() :
        font_library_(),
        font_manager_(font_library_,
                      freetype_engine::get_mapping(),
                      freetype_engine::get_cache())
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

    const value_type * get(glyph_info const & glyph);
    const value_type * get_halo(glyph_info const & glyph, double halo_radius);

private:
    std::unordered_map<glyph_cache_key, value_type> cache_;
    std::unordered_map<glyph_halo_cache_key, value_type> halo_cache_;

    font_library font_library_;
    face_manager_freetype font_manager_;

    const value_type * render(glyph_cache_key const & key,
                            glyph_info const & glyph);
    FT_Error select_closest_size(glyph_info const& glyph, FT_Face & face) const;

    const value_type * render_halo(
        glyph_halo_cache_key const & key,
        glyph_info const & glyph,
        double halo_radius);

    void render_halo(
        img_type & dst,
        FT_Bitmap const & src,
        int radius) const;
};

template <typename T>
class agg_text_renderer : public text_renderer
{
public:
    using pixmap_type = T;
    agg_text_renderer (pixmap_type & pixmap, halo_rasterizer_e halo_rasterizer,
                       rasterizer const & ras,
                       composite_mode_e comp_op = src_over,
                       composite_mode_e halo_comp_op = src_over,
                       double scale_factor = 1.0,
                       stroker_ptr stroker = stroker_ptr());
    void render(glyph_positions const& positions);
private:
    pixmap_type & pixmap_;
    halo_cache halo_cache_;
    glyph_cache glyph_cache_;
    rasterizer const & ras_;

    void render_halo(unsigned char *buffer,
                     unsigned width,
                     unsigned height,
                     unsigned pixel_width,
                     unsigned rgba, int x, int y,
                     double halo_radius, double opacity,
                     composite_mode_e comp_op);
};

}
#endif // RENDERER_HPP
