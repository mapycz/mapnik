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

#include <mapnik/box2d.hpp>
#include <mapnik/pixel_position.hpp>
#include <mapnik/text/text_layout.hpp>
#include <mapnik/text/glyph_positions.hpp>
#include <mapnik/text/rotation.hpp>
#include <mapnik/util/noncopyable.hpp>
#include <mapnik/extend_converter.hpp>
#include <mapnik/vertex_cache.hpp>
#include <mapnik/text/point_layout.hpp>
#include <mapnik/text/text_line_policy.hpp>
#include <mapnik/vertex_converters.hpp>
#include <mapnik/vertex_adapters.hpp>
#include <mapnik/label_placements/line_layout.hpp>
#include <mapnik/label_placements/point_position_accessor.hpp>

namespace mapnik
{

static const double halign_adjust_extend = 1000;

template <typename SubLayout>
class text_extend_line_layout : util::noncopyable
{
public:
    using box_type = box2d<double>;
    using params_type = label_placement::placement_params;

    text_extend_line_layout(params_type const & params);

    template <typename LayoutGenerator, typename Geom>
    bool try_placement(
        LayoutGenerator & layout_generator,
        Geom & geom);

    // TODO: needed?
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
template <typename LayoutGenerator, typename Geom>
bool text_extend_line_layout<SubLayout>::try_placement(
    LayoutGenerator & layout_generator,
    Geom & geom)
{
    layout_container & layouts = *layout_generator.layouts_;

    if (!layouts.line_count()) return true;

    horizontal_alignment_e halign = layouts.root_layout().horizontal_alignment();
    if (halign == H_ADJUST)
    {
        extend_converter<Geom> ec(geom, halign_adjust_extend);
        return sublayout_.try_placement(layout_generator, ec);
    }

    return sublayout_.try_placement(layout_generator, geom);
}

// ===========================================

template <typename SubLayout>
class text_line_layout : label_placement::line_layout<SubLayout>
{
public:
    using label_placement::line_layout<SubLayout>::line_layout;

    template <typename LayoutGenerator, typename Geom>
    bool try_placement(
        LayoutGenerator & layout_generator,
        Geom & geom)
    {
        vertex_cache path(geom);
        double layout_width = this->sublayout_.get_length(layout_generator);
        text_line_policy policy(path,
            layout_generator, layout_width, this->params_);
        return label_placement::line_layout<SubLayout>::try_placement(
            layout_generator, path, policy);
    }
};

// ===========================================

class single_line_layout : util::noncopyable
{
public:
    using box_type = box2d<double>;
    using params_type = label_placement::placement_params;
    using layout_generator_type = text_layout_generator;
    using detector_type = layout_generator_type::detector_type;

    single_line_layout(params_type const & params);

    bool try_placement(
        text_layout_generator & layout_generator,
        vertex_cache & path);

    inline double get_length(text_layout_generator const & layout_generator) const
    {
        return layout_generator.layouts_->width();
    }

private:
    bool try_placement(
        layout_container const & layouts,
        detector_type & detector,
        evaluated_text_properties const & text_props,
        vertex_cache &pp,
        text_upright_e orientation,
        glyph_positions & glyphs);

    void process_bboxes(
        detector_type & detector,
        layout_container & layouts,
        glyph_positions_ptr & glyphs);

    bool collision(
        detector_type & detector,
        evaluated_text_properties const & text_props,
        box_type const& box,
        const value_unicode_string &repeat_key) const;

    bool is_reachable(
        detector_type const & detector,
        vertex_cache const & path,
        text_layout const & layout) const;

    params_type const & params_;
    const std::vector<std::string> collision_cache_insert_;
};

}//ns mapnik

#endif // LINE_LAYOUT_HPP
