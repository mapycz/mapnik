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

#ifndef MAPNIK_FONT_ENGINE_FREETYPE_HPP
#define MAPNIK_FONT_ENGINE_FREETYPE_HPP

// mapnik
#include <mapnik/config.hpp>
#include <mapnik/util/singleton.hpp>
#include <mapnik/util/noncopyable.hpp>
#include <mapnik/font_set.hpp>
#include <mapnik/text/font_library.hpp>

// stl
#include <memory>
#include <unordered_map>
#include <utility> // pair
#include <vector>

namespace boost { template <class T> class optional; }

namespace mapnik
{

class stroker;
class font_face_set;
using face_set_ptr = std::unique_ptr<font_face_set>;
class font_face;
using face_ptr = std::shared_ptr<font_face>;

class glyph_cache;
//class face_manager;

class MAPNIK_DECL freetype_engine : public singleton<freetype_engine, CreateUsingNew>,
                                    private util::noncopyable
{
    friend class CreateUsingNew<freetype_engine>;
    friend class Map;
public:
    using font_file_mapping_type = std::unordered_map<std::string,std::pair<int,std::string>>;
    using font_memory_cache_type = std::unordered_map<std::string, std::pair<std::unique_ptr<char[]>, std::size_t>>;

    static bool is_font_file(std::string const& file_name);
    static bool register_font(std::string const& file_name);
    static bool register_fonts(std::string const& dir, bool recurse = false);
    static std::vector<std::string> face_names();
    static font_file_mapping_type const& get_mapping();
    static font_memory_cache_type & get_cache();
    static bool can_open(std::string const& face_name,
                         font_library & library,
                         font_file_mapping_type const& font_file_mapping,
                         font_file_mapping_type const& global_font_file_mapping);

    static glyph_cache & get_glyph_cache();
    //static face_manager & get_face_manager();

    //face_ptr get_face(std::string const& name);
    static face_set_ptr get_face_set(std::string const& name);
    static face_set_ptr get_face_set(font_set const& fset);
    static face_set_ptr get_face_set(std::string const& name, boost::optional<font_set> fset);

    ~freetype_engine();

private:
    freetype_engine();

    bool is_font_file_impl(std::string const& file_name);
    std::vector<std::string> face_names_impl();
    font_file_mapping_type const& get_mapping_impl();
    font_memory_cache_type& get_cache_impl();
    glyph_cache & get_glyph_cache_impl();
    //face_manager & get_face_manager_impl();

    bool can_open_impl(std::string const& face_name,
                  font_library & library,
                  font_file_mapping_type const& font_file_mapping,
                  font_file_mapping_type const& global_font_file_mapping);

    face_ptr create_face(std::string const& face_name,
                         font_library & library,
                         font_file_mapping_type const& global_font_file_mapping,
                         freetype_engine::font_memory_cache_type & global_memory_fonts);
    bool register_font_impl(std::string const& file_name);
    bool register_fonts_impl(std::string const& dir, bool recurse);
    bool register_font_impl(std::string const& file_name, FT_LibraryRec_ * library);
    bool register_fonts_impl(std::string const& dir, FT_LibraryRec_ * library, bool recurse = false);
    bool register_font_impl(std::string const& file_name,
                            font_library & libary,
                            font_file_mapping_type & font_file_mapping);
    bool register_fonts_impl(std::string const& dir,
                             font_library & libary,
                             font_file_mapping_type & font_file_mapping,
                             bool recurse = false);

    face_ptr get_face_impl(std::string const& name);
    //face_set_ptr get_face_set_impl(std::string const& name);
    //face_set_ptr get_face_set_impl(font_set const& fset);
    //face_set_ptr get_face_set_impl(std::string const& name, boost::optional<font_set> fset);
    //stroker & get_stroker_impl() const { return *stroker_; }

    font_file_mapping_type global_font_file_mapping_;
    font_memory_cache_type global_memory_fonts_;
    font_library font_library_;
    

    std::unordered_map<std::string, face_ptr> face_cache_;
    std::unique_ptr<stroker> stroker_;

    //std::unique_ptr<face_manager> face_manager_;
    std::unique_ptr<glyph_cache> glyph_cache_;
};

/*
class MAPNIK_DECL face_manager
{
public:
    face_manager(font_library & library,
                 freetype_engine::font_file_mapping_type const& font_file_mapping,
                 freetype_engine::font_memory_cache_type const& font_cache,
                 std::mutex & mutex);
    face_ptr get_face(std::string const& name);
    face_set_ptr get_face_set(std::string const& name);
    face_set_ptr get_face_set(font_set const& fset);
    face_set_ptr get_face_set(std::string const& name, boost::optional<font_set> fset);
    stroker & get_stroker() const { return *stroker_; }

    face_manager(face_manager const&) = delete;

private:
    font_library & library_;
    freetype_engine::font_file_mapping_type const& font_file_mapping_;
    freetype_engine::font_memory_cache_type const& font_memory_cache_;
    std::unique_ptr<stroker> stroker_;
    std::mutex & mutex_;
};

using face_manager_freetype = face_manager;
*/
extern template class MAPNIK_DECL singleton<freetype_engine, CreateUsingNew>;

}

#endif // MAPNIK_FONT_ENGINE_FREETYPE_HPP
