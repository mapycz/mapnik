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

namespace mapnik
{

class point_layout : util::noncopyable
{
public:
    using box_type = box2d<double>;
    using params_type = label_placement::placement_params;

    point_layout(params_type const & params);

    template <typename Detector>
    bool try_placement(
        text_layout_generator & layout_generator,
        Detector & detector,
        pixel_position const& pos);

    template <typename Detector>
    bool try_placement(
        text_layout_generator & layout_generator,
        Detector & detector,
        point_position const& pos);

    inline double get_length(text_layout_generator const &) const
    {
        return 0;
    }

protected:
    template <typename Detector>
    bool try_placement(
        layout_container const & layouts,
        Detector & detector,
        evaluated_text_properties const & text_props,
        pixel_position const& pos,
        glyph_positions & glyphs,
        std::vector<box_type> & bboxes);

    template <typename Detector>
    void process_bboxes(
        Detector & detector,
        layout_container & layouts,
        glyph_positions_ptr & glyphs,
        std::vector<box_type> const & bboxes);

    template <typename Detector>
    bool collision(
        Detector & detector,
        evaluated_text_properties const & text_props,
        box_type const& box,
        const value_unicode_string &repeat_key,
        bool line_placement) const;

    params_type const & params_;
    const std::vector<std::string> collision_cache_insert_;
    const std::vector<std::string> collision_cache_detect_;
};

class shield_layout : public point_layout
{
public:
    shield_layout(params_type const & params);

    template <typename Detector>
    bool try_placement(
        text_layout_generator & layout_generator,
        Detector & detector,
        pixel_position const& pos);

    template <typename Detector>
    bool try_placement(
        text_layout_generator & layout_generator,
        Detector & detector,
        point_position const& pos);

private:
    template <typename Detector>
    bool add_marker(
        Detector & detector,
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
