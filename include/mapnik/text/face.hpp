/*****************************************************************************
 *
 * This file is part of Mapnik (c++ mapping toolkit)
 *
 * Copyright (C) 2013 Artem Pavlenko
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
#ifndef MAPNIK_FACE_HPP
#define MAPNIK_FACE_HPP

//mapnik
#include <mapnik/text/glyph_info.hpp>
#include <mapnik/config.hpp>
#include <mapnik/noncopyable.hpp>
#include <mapnik/text/text_properties.hpp>

// freetype2
extern "C"
{
#include <ft2build.h>
#include FT_FREETYPE_H
#include FT_STROKER_H
}

//stl
#include <unordered_map>
#include <memory>
#include <string>
#include <vector>
#include <mutex>

namespace mapnik
{

class font_face : mapnik::noncopyable
{
public:
    font_face(FT_Face face);

    std::string family_name() const
    {
        return std::string(face_->family_name);
    }

    std::string style_name() const
    {
        return std::string(face_->style_name);
    }

    FT_Face get_face() const
    {
        return face_;
    }

    double get_char_height(double size, evaluated_format_properties_ptr const& f) const;

    bool set_character_sizes(double size);
    bool set_unscaled_character_sizes();

    bool glyph_dimensions(glyph_info &glyph) const;

    ~font_face();

private:
    FT_Face face_;

    using height_cache_map = std::map<double, double>;
    mutable height_cache_map height_cache_;
    mutable std::mutex height_cache_mutext_;
};
using face_ptr = std::shared_ptr<font_face>;


class MAPNIK_DECL font_face_set : private mapnik::noncopyable
{
public:
    using iterator = std::vector<face_ptr>::iterator;
    font_face_set(void) : faces_(){}

    void add(face_ptr face);
    void set_character_sizes(double size);
    void set_unscaled_character_sizes();

    unsigned size() const { return faces_.size(); }
    iterator begin() { return faces_.begin(); }
    iterator end() { return faces_.end(); }
private:
    std::vector<face_ptr> faces_;
};
using face_set_ptr = std::unique_ptr<font_face_set>;


// FT_Stroker wrapper
class stroker : mapnik::noncopyable
{
public:
    explicit stroker(FT_Stroker s)
        : s_(s) {}
    ~stroker();

    void init(double radius);
    FT_Stroker const& get() const { return s_; }
private:
    FT_Stroker s_;
};

} //ns mapnik

#endif // FACE_HPP
