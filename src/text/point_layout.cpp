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
#include <mapnik/text/placement_finder_impl.hpp>
#include <mapnik/text/placements/base.hpp>
#include <mapnik/text/text_layout.hpp>
#include <mapnik/text/glyph_info.hpp>
#include <mapnik/text/text_properties.hpp>
#include <mapnik/text/glyph_positions.hpp>
#include <mapnik/vertex_cache.hpp>
#include <mapnik/util/math.hpp>
#include <mapnik/tolerance_iterator.hpp>

// stl
#include <vector>

namespace mapnik
{

point_layout::point_layout(feature_impl const& feature,
                                   attributes const& attr,
                                   DetectorType &detector,
                                   box2d<double> const& extent,
                                   text_placement_info const& info,
                                   face_manager_freetype & font_manager,
                                   double scale_factor)
    : feature_(feature),
      attr_(attr),
      detector_(detector),
      extent_(extent),
      text_props_(evaluate_text_properties(info.properties,feature_,attr_)),
      scale_factor_(scale_factor),
      font_manager_(font_manager),
      has_marker_(false),
      marker_(),
      marker_box_(),
      marker_unlocked_(false),
      marker_displacement_(),
      layouts_(std::make_unique<layout_container>(
         std::move(std::make_unique<text_layout>(font_manager_,
                                                 feature_,
                                                 attr_,
                                                 scale_factor_,
                                                 info.properties,
                                                 info.properties.layout_defaults,
                                                 info.properties.format_tree())))),
      move_dx_(layouts_->root_layout().displacement().x)
{
}

glyph_positions_ptr point_layout::get_placement(pixel_position const& pos)
{
    glyph_positions_ptr glyphs = std::make_unique<glyph_positions>();
    std::vector<box2d<double> > bboxes;

    glyphs->reserve(layouts_->glyphs_count());
    bboxes.reserve(layouts_->size());

    bool base_point_set = false;
    for (auto const& layout_wrapper : *layouts_)
    {
        text_layout const& layout = layout_wrapper.get();
        rotation const& orientation = layout.orientation();

        // Find text origin.
        pixel_position layout_center = pos + layout.displacement();

        if (!base_point_set)
        {
            glyphs->set_base_point(layout_center);
            base_point_set = true;
        }

        box2d<double> bbox = layout.bounds();
        bbox.re_center(layout_center.x, layout_center.y);

        /* For point placements it is faster to just check the bounding box. */
        if (collision(bbox, layouts_->text(), false)) return glyph_positions_ptr();

        if (layout.glyphs_count()) bboxes.push_back(std::move(bbox));

        pixel_position layout_offset = layout_center - glyphs->get_base_point();
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
                glyphs->emplace_back(glyph, (pixel_position(x, y).rotate(orientation)) + layout_offset, orientation);
                if (glyph.advance())
                {
                    //Only advance if glyph is not part of a multiple glyph sequence
                    x += glyph.advance() + glyph.format->character_spacing * scale_factor_;
                }
            }
        }
    }

    // add_marker first checks for collision and then updates the detector.
    if (has_marker_ && !add_marker(glyphs, pos, bboxes)) return glyph_positions_ptr();

    box2d<double> label_box;
    bool first = true;
    for (box2d<double> const& box : bboxes)
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
        detector_.insert(box, layouts_->text());
    }

    // do not render text off the canvas
    // TODO: throw away single glyphs earlier?
    if (!extent_.intersects(label_box))
    {
        glyphs->clear();
    }

    return glyphs;
}

bool point_layout::collision(const box2d<double> &box, const value_unicode_string &repeat_key, bool line_placement) const
{
    double margin, repeat_distance;
    if (line_placement)
    {
        margin = text_props_->margin * scale_factor_;
        repeat_distance = (text_props_->repeat_distance != 0 ? text_props_->repeat_distance : text_props_->minimum_distance) * scale_factor_;
    }
    else
    {
        margin = (text_props_->margin != 0 ? text_props_->margin : text_props_->minimum_distance) * scale_factor_;
        repeat_distance = text_props_->repeat_distance * scale_factor_;
    }
    return (text_props_->avoid_edges && !extent_.contains(box))
        ||
        (text_props_->minimum_padding > 0 &&
         !extent_.contains(box + (scale_factor_ * text_props_->minimum_padding)))
        ||
        (!text_props_->allow_overlap &&
         ((repeat_key.length() == 0 && !detector_.has_placement(box, margin))
          ||
          (repeat_key.length() > 0 && !detector_.has_placement(box, margin, repeat_key, repeat_distance))));
}

void point_layout::set_marker(marker_info_ptr m, box2d<double> box, bool marker_unlocked, pixel_position const& marker_displacement)
{
    marker_ = m;
    marker_box_ = box * scale_factor_;
    marker_displacement_ = marker_displacement * scale_factor_;
    marker_unlocked_ = marker_unlocked;
    has_marker_ = true;
}


bool point_layout::add_marker(glyph_positions_ptr & glyphs, pixel_position const& pos, std::vector<box2d<double>> & bboxes) const
{
    pixel_position real_pos = (marker_unlocked_ ? pos : glyphs->get_base_point()) + marker_displacement_;
    box2d<double> bbox = marker_box_;
    bbox.move(real_pos.x, real_pos.y);
    if (collision(bbox, layouts_->text(), false)) return false;
    detector_.insert(bbox);
    bboxes.push_back(std::move(bbox));
    glyphs->set_marker(marker_, real_pos);
    return true;
}

}// ns mapnik
