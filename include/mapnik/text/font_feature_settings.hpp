/*****************************************************************************
 *
 * This file is part of Mapnik (c++ mapping toolkit)
 *
 * Copyright (C) 2014 Artem Pavlenko
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

#ifndef MAPNIK_FONT_FEATURE_SETTINGS_HPP
#define MAPNIK_FONT_FEATURE_SETTINGS_HPP

// stl
#include <vector>
#include <memory>
#include <limits>

// harfbuzz
#include <harfbuzz/hb.h>

namespace mapnik
{

class font_feature_settings
{
public:
    using font_feature = hb_feature_t;
    using feature_vector = std::vector<font_feature>;
    using feature_iterator = feature_vector::iterator;

    font_feature_settings(std::string const& features);
    font_feature_settings();

    void from_string(std::string const& features);
    std::string to_string() const;

    void append(std::string const& feature);
    void append(font_feature const& feature) { features_.push_back(feature); }

    const font_feature* get_features() const { return features_.data(); }
    feature_vector::size_type count() const { return features_.size(); }

private:
    feature_vector features_;
};

using font_feature_settings_ptr = std::shared_ptr<font_feature_settings>;

constexpr unsigned int font_feature_range_global_start = 0u;
constexpr unsigned int font_feature_range_global_end = std::numeric_limits<unsigned int>::max();

constexpr hb_feature_t font_feature_liga_off = { HB_TAG('l', 'i', 'g', 'a'), 0, font_feature_range_global_start, font_feature_range_global_end };

} // mapnik namespace

#endif // MAPNIK_FONT_FEATURE_SETTINGS_HPP
