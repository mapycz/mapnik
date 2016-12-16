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
#include <mapnik/pixel_position.hpp>
#include <mapnik/font_engine_freetype.hpp>
#include <mapnik/text/placements/base.hpp>
#include <mapnik/text/text_layout.hpp>
#include <mapnik/label_placements/base.hpp>
#include <mapnik/text/text_line_policy.hpp>
#include <mapnik/text/line_layout.hpp>

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

    group_layout_generator(
        params_type const & params,
        face_manager_freetype & font_manager,
        text_placement_info & info,
        std::list<box_element> const & box_elements);

    bool next();
    void reset();

    inline evaluated_text_properties const & get_text_props() const
    {
        return *text_props_;
    }

    inline bool has_placements() const
    {
        return !results_.empty();
    }

    inline pixel_position_list & get_placements()
    {
        return results_;
    }

    inline multi_policy_enum multi_policy() const
    {
        return text_props_->largest_bbox_only ? LARGEST_MULTI : EACH_MULTI;
    }

    face_manager_freetype &font_manager_;
    text_placement_info & info_;
    evaluated_text_properties_ptr text_props_;
    std::unique_ptr<layout_container> layouts_;
    bool state_;
    pixel_position_list results_;
    std::list<box_element> box_elements_;
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
class group_line_layout : line_layout<SubLayout>
{
public:
    using params_type = label_placement::placement_params;

    group_line_layout(params_type const & params)
        : line_layout<SubLayout>(params)
    {
    }

    template <typename LayoutGenerator, typename Detector, typename Geom>
    bool try_placement(
        LayoutGenerator & layout_generator,
        Detector & detector,
        Geom & geom)
    {
        vertex_cache path(geom);
        group_line_policy policy(path, layout_generator, 0, this->params_);
        return line_layout<SubLayout>::try_placement(
            layout_generator, detector, path, policy);
    }
};

class group_point_layout : util::noncopyable
{
public:
    using box_type = box2d<double>;
    using params_type = label_placement::placement_params;

    group_point_layout(params_type const & params);

    template <typename Detector>
    bool try_placement(
        group_layout_generator & layout_generator,
        Detector & detector,
        pixel_position const& pos);

    template <typename Detector>
    bool try_placement(
        group_layout_generator & layout_generator,
        Detector & detector,
        point_position const& pos);

    inline double get_length(group_layout_generator const &) const
    {
        return 0;
    }

protected:
    template <typename Detector>
    bool collision(
        Detector & detector,
        evaluated_text_properties const & text_props,
        box_type const& box,
        const value_unicode_string &repeat_key) const;

    params_type const & params_;
};

} //namespace
#endif // GROUP_SYMBOLIZER_HELPER_HPP
