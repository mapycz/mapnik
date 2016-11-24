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
#ifndef MAPNIK_POINT_LAYOUT_HPP
#define MAPNIK_POINT_LAYOUT_HPP

//mapnik
#include <mapnik/box2d.hpp>
#include <mapnik/pixel_position.hpp>
#include <mapnik/text/text_layout.hpp>
#include <mapnik/text/glyph_positions.hpp>
#include <mapnik/text/rotation.hpp>
#include <mapnik/util/noncopyable.hpp>
#include <mapnik/extend_converter.hpp>
#include <mapnik/vertex_cache.hpp>

namespace mapnik
{

class label_collision_detector4;
using DetectorType = label_collision_detector4;

class feature_impl;
class text_placement_info;
struct glyph_info;

class text_layout_generator
{
}

class point_layout : util::noncopyable
{
public:
    using box_type = box2d<double>;

    point_layout(feature_impl const& feature,
                     attributes const& attr,
                     DetectorType & detector,
                     box_type const& extent,
                     text_placement_info const& placement_info,
                     face_manager_freetype & font_manager,
                     double scale_factor);

    bool try_placement(pixel_position const& pos);

    std::unique_ptr<layout_container> & get_layouts()
    {
        return layouts_;
    }

protected:
    bool try_placement(pixel_position const& pos, glyph_positions & glyphs, std::vector<box_type> & bboxes);
    void process_bboxes(glyph_positions_ptr & glyphs, std::vector<box_type> & bboxes);
    bool collision(box_type const& box, const value_unicode_string &repeat_key, bool line_placement) const;

    feature_impl const& feature_;
    attributes const& attr_;
    DetectorType & detector_;
    box_type const& extent_;
    const evaluated_text_properties_ptr text_props_;
    const double scale_factor_;
    face_manager_freetype &font_manager_;
    std::unique_ptr<layout_container> layouts_;
};

class shield_layout : point_layout
{
public:
    shield_layout(feature_impl const& feature,
                     attributes const& attr,
                     DetectorType & detector,
                     box_type const& extent,
                     text_placement_info const& placement_info,
                     face_manager_freetype & font_manager,
                     double scale_factor,
                     marker_info_ptr marker,
                     box_type marker_box,
                     bool marker_unlocked,
                     pixel_position const& marker_displacement);

    bool try_placement(pixel_position const& pos);

private:
    bool add_marker(glyph_positions & glyphs, pixel_position const& pos, std::vector<box_type> & bboxes) const;

    marker_info_ptr marker_;
    box_type marker_box_;
    bool marker_unlocked_;
    pixel_position marker_displacement_;
};

}//ns mapnik

#endif // MAPNIK_POINT_LAYOUT_HPP
