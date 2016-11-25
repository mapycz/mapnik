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
#ifndef LINE_LAYOUT_HPP
#define LINE_LAYOUT_HPP

//mapnik
#include <mapnik/box2d.hpp>
#include <mapnik/pixel_position.hpp>
#include <mapnik/text/text_layout.hpp>
#include <mapnik/text/glyph_positions.hpp>
#include <mapnik/text/rotation.hpp>
#include <mapnik/util/noncopyable.hpp>
#include <mapnik/extend_converter.hpp>
#include <mapnik/vertex_cache.hpp>
#include <mapnik/text/point_layout.hpp>
#include <mapnik/tolerance_iterator.hpp>

namespace mapnik
{

class label_collision_detector4;
using DetectorType = label_collision_detector4;

class feature_impl;
class text_placement_info;
struct glyph_info;

static const double halign_adjust_extend = 1000;

template <typename SubLayout>
class line_layout : util::noncopyable
{
public:
    using box_type = box2d<double>;

    line_layout(SubLayout & sublayout,
                DetectorType & detector,
                box_type const& extent,
                double scale_factor);

    template <typename Geom>
    bool try_placement(
        text_layout_generator & layout_generator,
        Geom & geom);

private:
    bool try_placement(
        text_layout_generator & layout_generator,
        vertex_cache & pp);

    // Moves dx pixels but makes sure not to fall of the end.
    void path_move_dx(vertex_cache & pp, double dx);

    // Adjusts user defined spacing to place an integer number of labels.
    double get_spacing(
        double path_length,
        double label_spacing,
        double layout_width) const;

    SubLayout & sublayout_;
    DetectorType & detector_;
    box2d<double> const & dims_;
    double scale_factor_;
};

template <typename SubLayout>
line_layout<SubLayout>::line_layout(
    SubLayout & sublayout,
    DetectorType & detector,
    box_type const& extent,
    double scale_factor)
    : sublayout_(sublayout),
      detector_(detector),
      dims_(extent),
      scale_factor_(scale_factor)
{
}

template <typename SubLayout> template <typename Geom>
bool line_layout<SubLayout>::try_placement(text_layout_generator & layout_generator, Geom & geom)
{
    layout_container & layouts = *layout_generator.get_layouts();

    if (!layouts.line_count()) return true;

    horizontal_alignment_e halign = layouts.root_layout().horizontal_alignment();
    if (halign == H_ADJUST)
    {
        extend_converter<Geom> ec(geom, halign_adjust_extend);
        vertex_cache pp(ec);
        return try_placement(layout_generator, pp);
    }

    vertex_cache pp(geom);
    return try_placement(layout_generator, pp);
}

template <typename SubLayout>
bool line_layout<SubLayout>::try_placement(text_layout_generator & layout_generator, vertex_cache & pp)
{
    bool success = false;
    evaluated_text_properties const & text_props = layout_generator.get_text_props();
    while (pp.next_subpath())
    {
        layout_container const & layouts = *layout_generator.get_layouts();

        if (pp.length() < text_props.minimum_path_length * scale_factor_ ||
            pp.length() < sublayout_.get_length(layouts))
        {
            continue;
        }

        double spacing = get_spacing(pp.length(), text_props.label_spacing,
            sublayout_.get_length(layouts));
        horizontal_alignment_e halign = layouts.root_layout().horizontal_alignment();

        // halign == H_LEFT -> don't move
        if (halign == H_MIDDLE ||
            halign == H_AUTO)
        {
            if (!pp.forward(spacing / 2.0)) continue;
        }
        else if (halign == H_RIGHT)
        {
            if (!pp.forward(pp.length())) continue;
        }
        else if (halign == H_ADJUST)
        {
            spacing = pp.length();
            if (!pp.forward(spacing / 2.0)) continue;
        }

        double move_dx = layouts.root_layout().displacement().x;
        if (move_dx != 0.0) path_move_dx(pp, move_dx);

        do
        {
            tolerance_iterator<exponential_function> tolerance_offset(
                text_props.label_position_tolerance * scale_factor_, spacing);
            while (tolerance_offset.next())
            {
                vertex_cache::scoped_state state(pp);
                if (pp.move(tolerance_offset.get()) &&
                    sublayout_.try_placement(layout_generator, pp))
                {
                    success = true;
                    break;
                }
            }
        } while (pp.forward(spacing));
    }
    return success;
}

template <typename SubLayout>
void line_layout<SubLayout>::path_move_dx(vertex_cache & pp, double dx)
{
    vertex_cache::state state = pp.save_state();
    if (!pp.move(dx)) pp.restore_state(state);
}

template <typename SubLayout>
double line_layout<SubLayout>::get_spacing(
    double path_length,
    double label_spacing,
    double layout_width) const
{
    int num_labels = 1;
    if (label_spacing > 0)
    {
        num_labels = static_cast<int>(std::floor(
            path_length / (label_spacing * scale_factor_ + layout_width)));
    }
    if (num_labels <= 0)
    {
        num_labels = 1;
    }
    return path_length / num_labels;
}


class single_line_layout : util::noncopyable
{
public:
    using box_type = box2d<double>;

    single_line_layout(
        DetectorType & detector,
        box_type const& extent,
        double scale_factor);

    bool try_placement(
        text_layout_generator & layout_generator,
        vertex_cache & path);

    inline double get_length(layout_container const & layouts) const
    {
        return layouts.width();
    }

private:
    bool try_placement(
        layout_container const & layouts,
        evaluated_text_properties const & text_props,
        vertex_cache &pp,
        text_upright_e orientation,
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
    box2d<double> const& dims_;
    double scale_factor_;
};

}//ns mapnik

#endif // LINE_LAYOUT_HPP
