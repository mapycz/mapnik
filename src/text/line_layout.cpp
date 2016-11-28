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
#include <mapnik/label_collision_detector.hpp>
#include <mapnik/view_transform.hpp>
#include <mapnik/expression_evaluator.hpp>
#include <mapnik/text/line_layout.hpp>
#include <mapnik/text/placements/base.hpp>
#include <mapnik/text/text_layout.hpp>
#include <mapnik/text/glyph_info.hpp>
#include <mapnik/text/text_properties.hpp>
#include <mapnik/text/glyph_positions.hpp>
#include <mapnik/vertex_cache.hpp>
#include <mapnik/util/math.hpp>

// stl
#include <vector>

namespace mapnik
{
namespace {
box2d<double> get_bbox(text_layout const& layout, glyph_info const& glyph, pixel_position const& pos, rotation const& rot)
{
    /*

      (0/ymax)           (width/ymax)
      ***************
      *             *
      (0/0)*             *
      *             *
      ***************
      (0/ymin)          (width/ymin)
      Add glyph offset in y direction, but not in x direction (as we use the full cluster width anyways)!
    */
    double width = layout.cluster_width(glyph.char_index);
    if (glyph.advance() <= 0) width = -width;
    pixel_position tmp, tmp2;
    tmp.set(0, glyph.ymax());
    tmp = tmp.rotate(rot);
    tmp2.set(width, glyph.ymax());
    tmp2 = tmp2.rotate(rot);
    box2d<double> bbox(tmp.x,  -tmp.y,
                       tmp2.x, -tmp2.y);
    tmp.set(width, glyph.ymin());
    tmp = tmp.rotate(rot);
    bbox.expand_to_include(tmp.x, -tmp.y);
    tmp.set(0, glyph.ymin());
    tmp = tmp.rotate(rot);
    bbox.expand_to_include(tmp.x, -tmp.y);
    pixel_position pos2 = pos + pixel_position(0, glyph.offset.y).rotate(rot);
    bbox.move(pos2.x , -pos2.y);
    return bbox;
}

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
} // anonymous namespace

single_line_layout::single_line_layout(
    DetectorType & detector,
    box2d<double> const & extent,
    double scale_factor,
    symbolizer_base const& sym,
    feature_impl const& feature,
    attributes const& vars)
    : detector_(detector),
      dims_(extent),
      scale_factor_(scale_factor)
{
}

bool single_line_layout::try_placement(
    text_layout_generator & layout_generator,
    vertex_cache & path)
{
    glyph_positions_ptr glyphs = std::make_unique<glyph_positions>();
    std::vector<box_type> bboxes;
    layout_container & layouts = *layout_generator.get_layouts();
    evaluated_text_properties const & text_props = layout_generator.get_text_props();

    // TODO: useful?
    //glyphs->reserve(layouts.glyphs_count());
    //bboxes.reserve(layouts.glyphs_count());

    if (try_placement(layouts, text_props, path, text_props.upright, *glyphs, bboxes))
    {
        process_bboxes(layouts, glyphs, bboxes);
        return true;
    }

    return false;
}

bool single_line_layout::try_placement(
    layout_container const & layouts,
    evaluated_text_properties const & text_props,
    vertex_cache &pp,
    text_upright_e orientation,
    glyph_positions & glyphs,
    std::vector<box_type> & bboxes)
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
                    if (adjust)
                    {
                        if (!off_pp.move(sign * (layout.cluster_width(current_cluster) + last_glyph_spacing)))
                        {
                            return false;
                        }
                        last_glyph_spacing = adjust_character_spacing;
                    }
                    else
                    {
                        if (!off_pp.move_to_distance(sign * (layout.cluster_width(current_cluster) + last_glyph_spacing)))
                        {
                            return false;
                        }
                        last_glyph_spacing = glyph.format->character_spacing * scale_factor_;
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
                if (collision(text_props, bbox, layouts.text(), true)) return false;
                bboxes.push_back(std::move(bbox));
                glyphs.emplace_back(glyph, pos, rot);
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
            bboxes.clear();
            return try_placement(layouts, text_props, pp,
                real_orientation == UPRIGHT_RIGHT ? UPRIGHT_LEFT : UPRIGHT_RIGHT,
                glyphs, bboxes);
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
        bboxes.clear();
        return try_placement(layouts, text_props, pp,
            real_orientation == UPRIGHT_RIGHT ? UPRIGHT_LEFT : UPRIGHT_RIGHT,
            glyphs, bboxes);
    }

    return true;
}

void single_line_layout::process_bboxes(
    layout_container & layouts,
    glyph_positions_ptr & glyphs,
    std::vector<box_type> const & bboxes)
{
    box_type label_box;
    bool first = true;
    for (auto const & box : bboxes)
    {
        if (first)
        {
            label_box = box;
            first = false;
        }
        else
        {
            label_box.expand_to_include(box);
        }
        detector_.insert(box, layouts.text());
    }

    // do not render text off the canvas
    // TODO: throw away single glyphs earlier?
    if (dims_.intersects(label_box))
    {
        layouts.placements_.emplace_back(std::move(glyphs));
    }
}

bool single_line_layout::collision(
    evaluated_text_properties const & text_props,
    const box2d<double> &box,
    const value_unicode_string &repeat_key,
    bool line_placement) const
{
    double margin, repeat_distance;
    if (line_placement)
    {
        margin = text_props.margin * scale_factor_;
        repeat_distance = (text_props.repeat_distance != 0 ? text_props.repeat_distance : text_props.minimum_distance) * scale_factor_;
    }
    else
    {
        margin = (text_props.margin != 0 ? text_props.margin : text_props.minimum_distance) * scale_factor_;
        repeat_distance = text_props.repeat_distance * scale_factor_;
    }
    return (text_props.avoid_edges && !dims_.contains(box))
        ||
        (text_props.minimum_padding > 0 &&
         !dims_.contains(box + (scale_factor_ * text_props.minimum_padding)))
        ||
        (!text_props.allow_overlap &&
         ((repeat_key.length() == 0 && !detector_.has_placement(box, margin))
          ||
          (repeat_key.length() > 0 && !detector_.has_placement(box, margin, repeat_key, repeat_distance))));
}

}// ns mapnik
