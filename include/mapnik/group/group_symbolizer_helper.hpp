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
#ifndef GROUP_SYMBOLIZER_HELPER_HPP
#define GROUP_SYMBOLIZER_HELPER_HPP

#include <mapnik/value_types.hpp>
#include <mapnik/vertex_cache.hpp>
#include <mapnik/collision_cache.hpp>
#include <mapnik/pixel_position.hpp>
#include <mapnik/font_engine_freetype.hpp>
#include <mapnik/text/placements/base.hpp>
#include <mapnik/text/text_layout.hpp>
#include <mapnik/label_placements/base.hpp>
#include <mapnik/text/text_line_policy.hpp>
#include <mapnik/label_placements/line_layout.hpp>

#include <list>

namespace mapnik {

using pixel_position_list = std::list<pixel_position>;

struct box_element
{
    box_element(box2d<double> const& box, value_unicode_string const& repeat_key = "")
       : box_(box),
         repeat_key_(repeat_key)
    {}
    box2d<double> box_;
    value_unicode_string repeat_key_;
};

struct group_layout_generator : util::noncopyable
{
    using params_type = label_placement::placement_params;
    using detector_type = keyed_collision_cache<label_collision_detector4>;

    group_layout_generator(
        params_type const & params,
        detector_type & detector,
        face_manager_freetype & font_manager,
        text_placement_info & info,
        std::list<box_element> const & box_elements);

    inline evaluated_text_properties const & get_text_props() const
    {
        return *text_props_;
    }

    inline multi_policy_enum multi_policy() const
    {
        return text_props_->largest_bbox_only ? LARGEST_MULTI : EACH_MULTI;
    }

    face_manager_freetype &font_manager_;
    evaluated_text_properties_ptr text_props_;
    pixel_position_list placements_;
    std::list<box_element> box_elements_;
    detector_type & detector_;
};

struct group_line_policy : text_line_policy<group_layout_generator>
{
    using text_line_policy<group_layout_generator>::text_line_policy;

    inline bool align()
    {
        return path_.forward(this->get_spacing() / 2.0);
    }
};

template <typename SubLayout>
class group_line_layout : label_placement::line_layout<SubLayout>
{
public:
    using params_type = label_placement::placement_params;
    using layout_generator_type = group_layout_generator;
    using detector_type = layout_generator_type::detector_type;

    group_line_layout(params_type const & params)
        : label_placement::line_layout<SubLayout>(params)
    {
    }

    template <typename Geom>
    bool try_placement(
        group_layout_generator & layout_generator,
        Geom & geom)
    {
        vertex_cache path(geom);
        group_line_policy policy(path, layout_generator, 0, this->params_);
        return label_placement::line_layout<SubLayout>::try_placement(
            layout_generator, path, policy);
    }
};

struct group_max_angle_line_policy : text_max_line_angle_policy<group_layout_generator>
{
    using text_max_line_angle_policy<group_layout_generator>::text_max_line_angle_policy;

    inline bool align()
    {
        return path_.forward(this->get_spacing() / 2.0);
    }
};

template <typename SubLayout>
class group_max_angle_line_layout : label_placement::line_layout<SubLayout>
{
public:
    using params_type = label_placement::placement_params;
    using layout_generator_type = group_layout_generator;
    using detector_type = layout_generator_type::detector_type;

    group_max_angle_line_layout(params_type const & params)
        : label_placement::line_layout<SubLayout>(params),
          max_angle_diff_((M_PI / 180.0) * params.get<value_double, keys::max_line_angle>()),
          max_angle_distance_(params.get_optional<value_double, keys::max_line_angle_distance>())

    {
    }

    template <typename Geom>
    bool try_placement(
        group_layout_generator & layout_generator,
        Geom & geom)
    {
        double layout_width = this->sublayout_.get_length(layout_generator);
        double distance = max_angle_distance_ ?
            (this->params_.scale_factor * *max_angle_distance_) :
            layout_width;
        vertex_cache path(geom);
        group_max_angle_line_policy policy(path, layout_generator,
            layout_width, this->params_, max_angle_diff_, distance);
        return label_placement::line_layout<SubLayout>::try_placement(
            layout_generator, path, policy);
    }

protected:
    const double max_angle_diff_;
    const boost::optional<double> max_angle_distance_;
};

class group_point_layout : util::noncopyable
{
public:
    using box_type = box2d<double>;
    using params_type = label_placement::placement_params;
    using layout_generator_type = group_layout_generator;
    using detector_type = layout_generator_type::detector_type;

    group_point_layout(params_type const & params);

    bool try_placement(
        group_layout_generator & layout_generator,
        pixel_position const& pos);

    bool try_placement(
        group_layout_generator & layout_generator,
        point_position const& pos);

    inline double get_length(group_layout_generator const &) const
    {
        return 0;
    }

protected:
    bool collision(
        detector_type & detector,
        evaluated_text_properties const & text_props,
        box_type const& box,
        const value_unicode_string &repeat_key) const;

    params_type const & params_;
    const std::vector<std::string> collision_cache_insert_;
    const std::vector<std::string> collision_cache_detect_;
};

} //namespace
#endif // GROUP_SYMBOLIZER_HELPER_HPP
