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
#include <mapnik/text/text_layout_generator.hpp>

namespace mapnik
{

class label_collision_detector4;
using DetectorType = label_collision_detector4;

class feature_impl;
struct glyph_info;

class point_layout : util::noncopyable
{
public:
    using box_type = box2d<double>;

    point_layout(
        DetectorType & detector,
        box_type const& extent,
        double scale_factor,
        symbolizer_base const& sym,
        feature_impl const& feature,
        attributes const& vars);

    bool try_placement(
        text_layout_generator & layout_generator,
        pixel_position const& pos);

    inline double get_length(text_layout_generator const &) const
    {
        return 0;
    }

protected:
    bool try_placement(
        layout_container const & layouts,
        evaluated_text_properties const & text_props,
        pixel_position const& pos,
        glyph_positions & glyphs,
        std::vector<box_type> & bboxes);
    void process_bboxes(
        layout_container & layouts,
        glyph_positions_ptr & glyphs,
        std::vector<box_type> const & bboxes);
    bool collision(
        evaluated_text_properties const & text_props,
        box_type const& box,
        const value_unicode_string &repeat_key,
        bool line_placement) const;

    DetectorType & detector_;
    box_type const& dims_;
    const double scale_factor_;
};

class shield_layout : public point_layout
{
public:
    shield_layout(
        DetectorType & detector,
        box_type const& extent,
        double scale_factor,
        symbolizer_base const& sym,
        feature_impl const& feature,
        attributes const& vars);

    bool try_placement(
        text_layout_generator & layout_generator,
        pixel_position const& pos);

private:
    bool add_marker(
        layout_container const & layouts,
        evaluated_text_properties const & text_props,
        glyph_positions & glyphs,
        pixel_position const& pos,
        std::vector<box_type> & bboxes) const;

    marker_info_ptr marker_;
    box_type marker_box_;
    const bool marker_unlocked_;
    const pixel_position marker_displacement_;
};

}//ns mapnik

#endif // MAPNIK_POINT_LAYOUT_HPP
