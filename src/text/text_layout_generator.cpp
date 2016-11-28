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
//mapnik
#include <mapnik/text/text_layout_generator.hpp>
#include <mapnik/text/placements/base.hpp>
#include <mapnik/text/text_layout.hpp>
#include <mapnik/text/text_properties.hpp>
#include <mapnik/make_unique.hpp>

// stl
#include <vector>

namespace mapnik
{

text_layout_generator::text_layout_generator(
    feature_impl const & feature,
    attributes const & vars,
    face_manager_freetype & font_manager,
    double scale_factor,
    text_placement_info & info)
    : feature_(feature),
      vars_(vars),
      font_manager_(font_manager),
      scale_factor_(scale_factor),
      info_(info),
      text_props_(evaluate_text_properties(info.properties, feature, vars))
{
}

bool text_layout_generator::next()
{
    if (!info_.next())
    {
        return false;
    }
    text_props_ = evaluate_text_properties(info_.properties, feature_, vars_);
    layouts_ = std::make_unique<layout_container>(
        std::move(std::make_unique<text_layout>(
            font_manager_,
            feature_,
            vars_,
            scale_factor_,
            info_.properties,
            info_.properties.layout_defaults,
            info_.properties.format_tree())));
    return true;
}

void text_layout_generator::reset()
{
    info_.reset_state();
}

}// ns mapnik
