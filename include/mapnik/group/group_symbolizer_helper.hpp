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

//mapnik
#include <mapnik/text/placements/base.hpp>
#include <mapnik/value_types.hpp>
#include <mapnik/pixel_position.hpp>
#include <mapnik/font_engine_freetype.hpp>
#include <mapnik/text/text_layout.hpp>

#include <list>

namespace mapnik {

class label_collision_detector4;
class feature_impl;
class proj_transform;
class view_transform;
class vertex_cache;
using DetectorType = label_collision_detector4;

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

/*
// Helper object that does some of the GroupSymbolizer placement finding work.
class group_symbolizer_helper
{
public:
    struct box_element
    {
        box_element(box2d<double> const& box, value_unicode_string const& repeat_key = "")
           : box_(box),
             repeat_key_(repeat_key)
        {}
        box2d<double> box_;
        value_unicode_string repeat_key_;
    };

    group_symbolizer_helper(group_symbolizer const& sym,
                            feature_impl const& feature,
                            attributes const& vars,
                            proj_transform const& prj_trans,
                            unsigned width,
                            unsigned height,
                            double scale_factor,
                            view_transform const& t,
                            DetectorType &detector,
                            box2d<double> const& query_extent,
                            symbol_cache const& sc);

    inline void add_box_element(box2d<double> const& box, value_unicode_string const& repeat_key = "")
    {
        box_elements_.push_back(box_element(box, repeat_key));
    }

    pixel_position_list const& get();

    // Iterate over the given path, placing line-following labels or point labels with respect to label_spacing.
    template <typename T>
    bool find_line_placements(T & path);

private:
    symbolizer_base const& sym_;
    feature_impl const& feature_;
    attributes const& vars_;
    proj_transform const& prj_trans_;
    view_transform const& t_;
    box2d<double> dims_;
    box2d<double> const& query_extent_;
    double scale_factor_;
    bool point_placement_;
    text_placement_info_ptr info_ptr_;

    // Check if a point placement fits at given position
    bool check_point_placement(pixel_position const& pos);
    // Checks for collision.
    bool collision(box2d<double> const& box, value_unicode_string const& repeat_key = "") const;
    double get_spacing(double path_length) const;

    DetectorType & detector_;

    // Boxes and repeat keys to take into account when finding placement.
    //  Boxes are relative to starting point of current placement.
    //
    std::list<box_element> box_elements_;

    pixel_position_list results_;
};*/

struct group_layout_generator : util::noncopyable
{
    group_layout_generator(
        feature_impl const& feature,
        attributes const& vars,
        face_manager_freetype & font_manager,
        double scale_factor,
        text_placement_info & info,
        std::list<box_element> const & box_elements);

    bool next();
    void reset();

    bool align(vertex_cache & path, double spacing) const;

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

    feature_impl const& feature_;
    attributes const& vars_;
    face_manager_freetype &font_manager_;
    const double scale_factor_;
    text_placement_info & info_;
    evaluated_text_properties_ptr text_props_;
    std::unique_ptr<layout_container> layouts_;
    bool state_;
    pixel_position_list results_;
    std::list<box_element> box_elements_;
};

class group_point_layout : util::noncopyable
{
public:
    using box_type = box2d<double>;

    group_point_layout(
        DetectorType & detector,
        box_type const& extent,
        double scale_factor,
        symbolizer_base const& sym,
        feature_impl const& feature,
        attributes const& vars);

    bool try_placement(
        group_layout_generator & layout_generator,
        pixel_position const& pos);

    inline double get_length(group_layout_generator const &) const
    {
        return 0;
    }

protected:
    bool collision(
        evaluated_text_properties const & text_props,
        box_type const& box,
        const value_unicode_string &repeat_key) const;

    DetectorType & detector_;
    box_type const& dims_;
    const double scale_factor_;
};

} //namespace
#endif // GROUP_SYMBOLIZER_HELPER_HPP
