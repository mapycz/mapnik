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

#include <mapnik/box2d.hpp>
#include <mapnik/pixel_position.hpp>
#include <mapnik/text/text_layout.hpp>
#include <mapnik/text/glyph_positions.hpp>
#include <mapnik/text/rotation.hpp>
#include <mapnik/util/noncopyable.hpp>
#include <mapnik/extend_converter.hpp>
#include <mapnik/vertex_cache.hpp>
#include <mapnik/text/text_layout_generator.hpp>
#include <mapnik/label_placements/base.hpp>

namespace mapnik { namespace text {

class point_layout : util::noncopyable
{
public:
    using box_type = box2d<double>;
    using params_type = label_placement::placement_params;
    using layout_generator_type = text_layout_generator;
    using detector_type = layout_generator_type::detector_type;

    point_layout(params_type const & params);

    bool try_placement(
        text_layout_generator & layout_generator,
        pixel_position const& pos);

    bool try_placement(
        text_layout_generator & layout_generator,
        point_position const& pos);

    inline double get_length(text_layout_generator const &) const
    {
        return 0;
    }

protected:
    bool try_placement(
        layout_container const & layouts,
        detector_type & detector,
        evaluated_text_properties const & text_props,
        pixel_position const& pos,
        glyph_positions & glyphs);

    bool process_bboxes(
        detector_type & detector,
        layout_container & layouts,
        glyph_positions_ptr & glyphs);

    bool collision(
        detector_type & detector,
        evaluated_text_properties const & text_props,
        box_type const& box,
        const value_unicode_string &repeat_key) const;

    params_type const & params_;
    const std::vector<std::string> collision_cache_insert_;
};

class shield_layout : public point_layout
{
public:
    shield_layout(params_type const & params);

    bool try_placement(
        text_layout_generator & layout_generator,
        pixel_position const& pos);

    bool try_placement(
        text_layout_generator & layout_generator,
        point_position const& pos);

private:
    bool add_marker(
        detector_type & detector,
        layout_container const & layouts,
        evaluated_text_properties const & text_props,
        glyph_positions & glyphs,
        pixel_position const& pos) const;

    void process_bboxes(
        detector_type & detector,
        layout_container & layouts,
        glyph_positions_ptr & glyphs);

    marker_info_ptr marker_;
    const bool marker_unlocked_;
    const pixel_position marker_displacement_;
};

} }

#endif // MAPNIK_POINT_LAYOUT_HPP
