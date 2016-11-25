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
#include <mapnik/text/point_layout.hpp>
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

point_layout::point_layout(DetectorType &detector,
                           box2d<double> const& extent,
                           double scale_factor)
    : detector_(detector),
      dims_(extent),
      scale_factor_(scale_factor)
{
}

bool point_layout::try_placement(
    text_layout_generator & layout_generator,
    pixel_position const & pos)
{
    glyph_positions_ptr glyphs = std::make_unique<glyph_positions>();
    std::vector<box_type> bboxes;
    layout_container & layouts = *layout_generator.get_layouts();
    evaluated_text_properties const & text_props = layout_generator.get_text_props();

    // TODO: useful?
    //glyphs->reserve(layouts.glyphs_count());
    //bboxes.reserve(layouts.size());

    if (try_placement(layouts, text_props, pos, *glyphs, bboxes))
    {
        process_bboxes(layouts, glyphs, bboxes);
        return true;
    }

    return false;
}

bool point_layout::try_placement(
    layout_container const & layouts,
    evaluated_text_properties const & text_props,
    pixel_position const & pos,
    glyph_positions & glyphs,
    std::vector<box_type> & bboxes)
{
    bool base_point_set = false;
    for (auto const& layout_wrapper : layouts)
    {
        text_layout const& layout = layout_wrapper.get();
        rotation const& orientation = layout.orientation();

        // Find text origin.
        pixel_position layout_center = pos + layout.displacement();

        if (!base_point_set)
        {
            glyphs.set_base_point(layout_center);
            base_point_set = true;
        }

        box2d<double> bbox = layout.bounds();
        bbox.re_center(layout_center.x, layout_center.y);

        /* For point placements it is faster to just check the bounding box. */
        if (collision(text_props, bbox, layouts.text(), false)) return false;

        if (layout.glyphs_count()) bboxes.push_back(std::move(bbox));

        pixel_position layout_offset = layout_center - glyphs.get_base_point();
        layout_offset.y = -layout_offset.y;

        // IMPORTANT NOTE:
        //   x and y are relative to the center of the text
        //   coordinate system:
        //   x: grows from left to right
        //   y: grows from bottom to top (opposite of normal computer graphics)

        double x, y;

        // set for upper left corner of text envelope for the first line, top left of first character
        y = layout.height() / 2.0;

        for ( auto const& line : layout)
        {
            y -= line.height(); //Automatically handles first line differently
            x = layout.jalign_offset(line.width());

            for (auto const& glyph : line)
            {
                // place the character relative to the center of the string envelope
                glyphs.emplace_back(glyph, (pixel_position(x, y).rotate(orientation)) + layout_offset, orientation);
                if (glyph.advance())
                {
                    //Only advance if glyph is not part of a multiple glyph sequence
                    x += glyph.advance() + glyph.format->character_spacing * scale_factor_;
                }
            }
        }
    }
    return true;
}

void point_layout::process_bboxes(
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
        //glyphs->clear();
    }
}

bool point_layout::collision(
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

shield_layout::shield_layout(DetectorType & detector,
                             box_type const& extent,
                             double scale_factor,
                             marker_info_ptr marker,
                             box2d<double> marker_box,
                             bool marker_unlocked,
                             pixel_position const& marker_displacement)
    : point_layout(detector, extent, scale_factor),
      marker_(marker),
      marker_box_(marker_box * scale_factor),
      marker_displacement_(marker_displacement * scale_factor),
      marker_unlocked_(marker_unlocked)
{
}

bool shield_layout::try_placement(
    text_layout_generator & layout_generator,
    pixel_position const& pos)
{
    glyph_positions_ptr glyphs = std::make_unique<glyph_positions>();
    std::vector<box_type> bboxes;
    layout_container & layouts = *layout_generator.get_layouts();
    evaluated_text_properties const & text_props = layout_generator.get_text_props();

    // TODO: useful?
    //glyphs->reserve(layouts.glyphs_count());
    //bboxes.reserve(layouts.size());

    if (point_layout::try_placement(layouts, text_props, pos, *glyphs, bboxes))
    {
        if (add_marker(layouts, text_props, *glyphs, pos, bboxes))
        {
            process_bboxes(layouts, glyphs, bboxes);
            return true;
        }
    }

    return false;
}

bool shield_layout::add_marker(
    layout_container const & layouts,
    evaluated_text_properties const & text_props,
    glyph_positions & glyphs,
    pixel_position const& pos,
    std::vector<box_type> & bboxes) const
{
    pixel_position real_pos = (marker_unlocked_ ? pos :
        glyphs.get_base_point()) + marker_displacement_;
    box2d<double> bbox = marker_box_;
    bbox.move(real_pos.x, real_pos.y);
    if (collision(text_props, bbox, layouts.text(), false)) return false;
    detector_.insert(bbox);
    bboxes.push_back(std::move(bbox));
    glyphs.set_marker(marker_, real_pos);
    return true;
}

}// ns mapnik
