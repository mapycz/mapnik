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

template <typename Layout>
struct position_accessor
{
    template <typename T>
    static T & get(T & geom)
    {
        return geom;
    }
};

template <>
struct position_accessor<point_layout>
{
    static pixel_position const & get(vertex_cache const & geom)
    {
        return geom.current_position();
    }

    static pixel_position const & get(pixel_position const & geom)
    {
        return geom;
    }
};

template <>
struct position_accessor<shield_layout> : position_accessor<point_layout>
{
};

static const double halign_adjust_extend = 1000;

template <typename SubLayout>
class text_extend_line_layout : util::noncopyable
{
public:
    using box_type = box2d<double>;
    using params_type = label_placement::placement_params;

    text_extend_line_layout(params_type const & params);

    template <typename LayoutGenerator, typename Detector, typename Geom>
    bool try_placement(
        LayoutGenerator & layout_generator,
        Detector & detector,
        Geom & geom);

    inline double get_length(text_layout_generator const & layout_generator) const
    {
        return layout_generator.layouts_->width();
    }

private:
    SubLayout sublayout_;
    params_type const & params_;
};

template <typename SubLayout>
text_extend_line_layout<SubLayout>::text_extend_line_layout(
    params_type const & params)
    : sublayout_(params),
      params_(params)
{
}

template <typename SubLayout>
template <typename LayoutGenerator, typename Detector, typename Geom>
bool text_extend_line_layout<SubLayout>::try_placement(
    LayoutGenerator & layout_generator,
    Detector & detector,
    Geom & geom)
{
    layout_container & layouts = *layout_generator.layouts_;

    if (!layouts.line_count()) return true;

    horizontal_alignment_e halign = layouts.root_layout().horizontal_alignment();
    if (halign == H_ADJUST)
    {
        extend_converter<Geom> ec(geom, halign_adjust_extend);
        vertex_cache pp(ec);
        return sublayout_.try_placement(layout_generator, detector, pp);
    }

    vertex_cache pp(geom);
    return sublayout_.try_placement(layout_generator, detector, pp);
}

template <typename SubLayout>
class line_layout : util::noncopyable
{
public:
    using box_type = box2d<double>;
    using params_type = label_placement::placement_params;

    line_layout(params_type const & params);

    template <typename LayoutGenerator, typename Detector, typename Geom>
    bool try_placement(
        LayoutGenerator & layout_generator,
        Detector & detector,
        Geom & geom);

    template <typename LayoutGenerator, typename Detector>
    bool try_placement(
        LayoutGenerator & layout_generator,
        Detector & detector,
        vertex_cache & pp);

protected:
    template <typename LayoutGenerator, typename Detector>
    bool try_placement(
        LayoutGenerator & layout_generator,
        Detector & detector,
        vertex_cache & pp,
        double minimum_path_length,
        double label_spacing,
        double label_position_tolerance);

    // Adjusts user defined spacing to place an integer number of labels.
    double get_spacing(
        double path_length,
        double label_spacing,
        double layout_width) const;

    SubLayout sublayout_;
    params_type const & params_;
};

template <typename SubLayout>
line_layout<SubLayout>::line_layout(params_type const & params)
    : sublayout_(params),
      params_(params)
{
}

template <typename SubLayout> template
<typename LayoutGenerator, typename Detector, typename Geom>
bool line_layout<SubLayout>::try_placement(
    LayoutGenerator & layout_generator,
    Detector & detector,
    Geom & geom)
{
    vertex_cache pp(geom);
    return try_placement(layout_generator, detector, pp);
}

template <typename SubLayout>
template <typename LayoutGenerator, typename Detector>
bool line_layout<SubLayout>::try_placement(
    LayoutGenerator & layout_generator,
    Detector & detector,
    vertex_cache & pp)
{
    evaluated_text_properties const & text_props = layout_generator.get_text_props();
    return try_placement(layout_generator, detector, pp,
        text_props.minimum_path_length,
        text_props.label_spacing,
        text_props.label_position_tolerance);
}

template <typename SubLayout>
template <typename LayoutGenerator, typename Detector>
bool line_layout<SubLayout>::try_placement(
    LayoutGenerator & layout_generator,
    Detector & detector,
    vertex_cache & pp,
    double minimum_path_length,
    double label_spacing,
    double label_position_tolerance)
{
    bool success = false;
    while (pp.next_subpath())
    {
        double layout_length = sublayout_.get_length(layout_generator);

        if (pp.length() < minimum_path_length * params_.scale_factor ||
            pp.length() < layout_length)
        {
            continue;
        }

        double spacing = get_spacing(pp.length(), label_spacing, layout_length);

        if (!layout_generator.align(pp, spacing))
        {
            continue;
        }

        do
        {
            tolerance_iterator<exponential_function> tolerance_offset(
                label_position_tolerance * params_.scale_factor, spacing);
            while (tolerance_offset.next())
            {
                vertex_cache::scoped_state state(pp);
                if (pp.move(tolerance_offset.get()) &&
                    sublayout_.try_placement(layout_generator, detector,
                        position_accessor<SubLayout>::get(pp)))
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
double line_layout<SubLayout>::get_spacing(
    double path_length,
    double label_spacing,
    double layout_width) const
{
    int num_labels = 1;
    if (label_spacing > 0)
    {
        num_labels = static_cast<int>(std::floor(
            path_length / (label_spacing * params_.scale_factor + layout_width)));
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
    using params_type = label_placement::placement_params;

    single_line_layout(params_type const & params);

    template <typename Detector>
    bool try_placement(
        text_layout_generator & layout_generator,
        Detector & detector,
        vertex_cache & path);

    inline double get_length(text_layout_generator const & layout_generator) const
    {
        return layout_generator.layouts_->width();
    }

private:
    template <typename Detector>
    bool try_placement(
        layout_container const & layouts,
        Detector & detector,
        evaluated_text_properties const & text_props,
        vertex_cache &pp,
        text_upright_e orientation,
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
};

}//ns mapnik

#endif // LINE_LAYOUT_HPP
