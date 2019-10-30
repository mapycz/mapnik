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
#include <mapnik/debug.hpp>
#include <mapnik/view_transform.hpp>
#include <mapnik/expression_evaluator.hpp>
#include <mapnik/text/line_layout.hpp>
#include <mapnik/text/placements/base.hpp>
#include <mapnik/text/text_layout.hpp>
#include <mapnik/text/glyph_info.hpp>
#include <mapnik/text/text_properties.hpp>
#include <mapnik/text/glyph_positions.hpp>
#include <mapnik/text/glyph_bbox.hpp>
#include <mapnik/vertex_cache.hpp>
#include <mapnik/util/math.hpp>

// stl
#include <vector>

namespace mapnik
{

text_upright_e simplify_upright(text_upright_e upright, double angle)
{
    if (upright == UPRIGHT_AUTO)
    {
        return (std::fabs(util::normalize_angle(angle)) > 0.5*M_PI) ? UPRIGHT_LEFT : UPRIGHT_RIGHT;
    }
    if (upright == UPRIGHT_AUTO_DOWN)
    {
        return (std::fabs(util::normalize_angle(angle)) < 0.5*M_PI) ? UPRIGHT_LEFT : UPRIGHT_RIGHT;
    }
    if (upright == UPRIGHT_LEFT_ONLY)
    {
        return UPRIGHT_LEFT;
    }
    if (upright == UPRIGHT_RIGHT_ONLY)
    {
        return  UPRIGHT_RIGHT;
    }
    return upright;
}

single_line_layout::single_line_layout(params_type const & params)
    : params_(params),
      collision_cache_insert_(parse_collision_detector_keys(
          params.get_optional<std::string, keys::collision_cache_insert>()))
{
}

bool single_line_layout::try_placement(
    text_layout_generator & layout_generator,
    vertex_cache & path)
{
    glyph_positions_ptr glyphs = std::make_unique<glyph_positions>();
    layout_container & layouts = *layout_generator.layouts_;
    evaluated_text_properties const & text_props = layout_generator.get_text_props();
    detector_type & detector = layout_generator.detector_;

    if (try_placement(layouts, detector, text_props, path, text_props.upright, *glyphs))
    {
        process_bboxes(detector, layouts, glyphs);
        return true;
    }

    return false;
}

bool single_line_layout::is_reachable(
    detector_type const & detector,
    vertex_cache const & path,
    text_layout const & layout) const
{
    double layout_max_dimension = std::max(layout.width(), layout.height());
    box2d<double> layout_max_box(0, 0, layout_max_dimension * 2, layout_max_dimension * 2);
    pixel_position pos = path.current_position();
    layout_max_box.re_center(pos.x, pos.y);
    return detector.extent().intersects(layout_max_box);
}

bool single_line_layout::try_placement(
    layout_container const & layouts,
    detector_type & detector,
    evaluated_text_properties const & text_props,
    vertex_cache &pp,
    text_upright_e orientation,
    glyph_positions & glyphs)
{
    //
    // IMPORTANT NOTE: See note about coordinate systems in find_point_placement()!
    //

    vertex_cache::scoped_state begin(pp);
    text_upright_e real_orientation = simplify_upright(orientation, pp.angle());
    unsigned upside_down_glyph_count = 0;

    for (auto const& layout_wrapper : layouts)
    {
        text_layout const& layout = layout_wrapper.get();

        if (!is_reachable(detector, pp, layout))
        {
            // Placements beyond collision extent are assumed to be successful.
            return true;
        }

        pixel_position align_offset = layout.alignment_offset();
        pixel_position const& layout_displacement = layout.displacement();
        double sign = (real_orientation == UPRIGHT_LEFT) ? -1 : 1;
        //double offset = 0 - (layout_displacement.y + 0.5 * sign * layout.height());
        double offset = layout_displacement.y - 0.5 * sign * layout.height();
        double adjust_character_spacing = .0;
        double layout_width = layout.width();
        bool adjust = layout.horizontal_alignment() == H_ADJUST;

        if (adjust)
        {
            text_layout::const_iterator longest_line = layout.longest_line();
            if (longest_line != layout.end())
            {
                double longest_line_offset = offset;
                for (text_layout::const_iterator line = layout.begin(); line != longest_line; line++)
                {
                    longest_line_offset += sign * line->height();
                }
                longest_line_offset += sign * longest_line->height() / 2.0;
                vertex_cache const & off_pp = pp.get_offseted(longest_line_offset, sign * layout_width);
                adjust_character_spacing = (off_pp.length() - longest_line->glyphs_width() - 2.0 * halign_adjust_extend) / longest_line->space_count();
                if (adjust_character_spacing < 0)
                {
                    return false;
                }
                layout_width = longest_line->glyphs_width() + longest_line->space_count() * adjust_character_spacing;
            }
        }

        for (auto const& line : layout)
        {
            // Only subtract half the line height here and half at the end because text is automatically
            // centered on the line
            offset += sign * line.height()/2;
            vertex_cache & off_pp = pp.get_offseted(offset, sign * layout_width);
            vertex_cache::scoped_state off_state(off_pp); // TODO: Remove this when a clean implementation in vertex_cache::get_offseted is done
            double line_width = adjust ? (line.glyphs_width() + line.space_count() * adjust_character_spacing) : line.width();

            if (!off_pp.move(sign * layout.jalign_offset(line_width) - align_offset.x)) return false;

            double last_cluster_angle = 999;
            int current_cluster = -1;
            pixel_position cluster_offset;
            double angle = 0;
            rotation rot;
            double last_glyph_spacing = 0.0;
            for (auto const& glyph : line)
            {
                if (current_cluster != static_cast<int>(glyph.char_index))
                {
                    const double cluster_width = (current_cluster < 0) ? 0 : layout.cluster_width(current_cluster);
                    const double distance = sign * (cluster_width + last_glyph_spacing);
                    if (adjust)
                    {
                        if (!off_pp.move(distance)) { return false; }
                        last_glyph_spacing = adjust_character_spacing;
                    }
                    else
                    {
                        if (!off_pp.move_to_distance(distance)) { return false; }
                        last_glyph_spacing = glyph.format->character_spacing * params_.scale_factor;
                    }
                    current_cluster = glyph.char_index;
                    // Only calculate new angle at the start of each cluster!
                    // Y axis is inverted.
                    angle = -util::normalize_angle(off_pp.angle(sign * layout.cluster_width(current_cluster)));
                    rot.init(angle);
                    if ((text_props.max_char_angle_delta > 0) && (last_cluster_angle != 999) &&
                        std::fabs(util::normalize_angle(angle - last_cluster_angle)) > text_props.max_char_angle_delta)
                    {
                        return false;
                    }
                    cluster_offset.clear();
                    last_cluster_angle = angle;
                }

                if (std::abs(angle) > M_PI/2)
                {
                    ++upside_down_glyph_count;
                }

                pixel_position pos = off_pp.current_position() + cluster_offset;
                // Center the text on the line
                double char_height = line.max_char_height();
                pos.y = -pos.y - char_height/2.0*rot.cos;
                pos.x =  pos.x + char_height/2.0*rot.sin;

                cluster_offset.x += rot.cos * glyph.advance();
                cluster_offset.y -= rot.sin * glyph.advance();

                box2d<double> bbox = get_bbox(layout, glyph, pos, rot);
                if (collision(detector, text_props, bbox, layouts.text())) return false;
                glyphs.emplace_back(glyph, pos, rot, bbox);
            }
            // See comment above
            offset += sign * line.height()/2;
        }
    }

    if (upside_down_glyph_count > static_cast<unsigned>(layouts.text().length() / 2))
    {
        if (orientation == UPRIGHT_AUTO)
        {
            // Try again with opposite orientation
            begin.restore();
            glyphs.clear();
            return try_placement(layouts, detector, text_props, pp,
                real_orientation == UPRIGHT_RIGHT ? UPRIGHT_LEFT : UPRIGHT_RIGHT, glyphs);
        }
        // upright==left-only or right-only and more than 50% of characters upside down => no placement
        else if (orientation == UPRIGHT_LEFT_ONLY || orientation == UPRIGHT_RIGHT_ONLY)
        {
            return false;
        }
    }
    else if (orientation == UPRIGHT_AUTO_DOWN)
    {
        // Try again with opposite orientation
        begin.restore();
        glyphs.clear();
        return try_placement(layouts, detector, text_props, pp,
            real_orientation == UPRIGHT_RIGHT ? UPRIGHT_LEFT : UPRIGHT_RIGHT,
            glyphs);
    }

    return true;
}

void single_line_layout::process_bboxes(
    detector_type & detector,
    layout_container & layouts,
    glyph_positions_ptr & glyphs)
{
    bool in_canvas = false;
    for (auto const & glyph_pos : *glyphs)
    {
        box2d<double> bbox(glyph_pos.bbox);
        detector.insert(bbox, layouts.text(), collision_cache_insert_);

        double halo_radius = glyph_pos.glyph.format->halo_radius * params_.scale_factor;
        bbox.pad(halo_radius);

        in_canvas |= params_.dims.intersects(bbox);
    }

    // do not render text off the canvas
    // TODO: throw away single glyphs earlier?
    if (in_canvas)
    {
        layouts.placements_.emplace_back(std::move(glyphs));
    }
}

bool single_line_layout::collision(
    detector_type & detector,
    evaluated_text_properties const & text_props,
    const box2d<double> &box,
    const value_unicode_string &repeat_key) const
{
    double margin = text_props.margin * params_.scale_factor;
    double repeat_distance = (text_props.repeat_distance != 0 ? text_props.repeat_distance : text_props.minimum_distance) * params_.scale_factor;
    return (text_props.avoid_edges && !params_.dims.contains(box))
        ||
        (text_props.minimum_padding > 0 &&
         !params_.dims.contains(box + (params_.scale_factor * text_props.minimum_padding)))
        ||
        (!text_props.allow_overlap &&
         ((repeat_key.length() == 0 && !detector.has_placement(box, margin, text_props.collision_cache_detect))
          ||
          (repeat_key.length() > 0 && !detector.has_placement(box, margin, repeat_key, repeat_distance, text_props.collision_cache_detect))));
}

}// ns mapnik
