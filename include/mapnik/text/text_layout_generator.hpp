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
#ifndef MAPNIK_TEXT_LAYOUT_GENERATOR
#define MAPNIK_TEXT_LAYOUT_GENERATOR

#include <mapnik/box2d.hpp>
#include <mapnik/pixel_position.hpp>
#include <mapnik/text/text_layout.hpp>
#include <mapnik/util/noncopyable.hpp>
#include <mapnik/extend_converter.hpp>
#include <mapnik/vertex_cache.hpp>
#include <mapnik/label_placements/base.hpp>

namespace mapnik
{

class feature_impl;
class text_placement_info;

struct text_layout_generator : util::noncopyable
{
    using params_type = label_placement::placement_params;

    text_layout_generator(
        params_type const & params,
        face_manager_freetype & font_manager,
        text_placement_info & info);

    bool next();
    void reset();

    inline evaluated_text_properties const & get_text_props() const
    {
        return *text_props_;
    }

    inline bool has_placements() const
    {
        return !layouts_->placements_.empty();
    }

    inline std::unique_ptr<layout_container> & get_placements()
    {
        return layouts_;
    }

    inline multi_policy_enum multi_policy() const
    {
        return text_props_->largest_bbox_only ? LARGEST_MULTI : EACH_MULTI;
    }

    params_type const & params_;
    face_manager_freetype &font_manager_;
    text_placement_info & info_;
    evaluated_text_properties_ptr text_props_;
    std::unique_ptr<layout_container> layouts_;
};

}//ns mapnik

#endif // MAPNIK_TEXT_LAYOUT_GENERATOR
