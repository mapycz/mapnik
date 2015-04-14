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

#ifndef TEXT_PLACEMENTS_ANGLE_HPP
#define TEXT_PLACEMENTS_ANGLE_HPP

#include <mapnik/text/placements/base.hpp>
#include <mapnik/tolerance_iterator.hpp>

namespace mapnik {

class text_placements_info_combined;
class feature_impl;
struct attribute;

class text_placements_angle: public text_placements
{
public:
    text_placements_angle(text_placements_ptr list_placement,
                          symbolizer_base::value_type const& angle,
                          symbolizer_base::value_type const& tolerance,
                          symbolizer_base::value_type const& step,
                          boost::optional<expression_ptr> const& anchor_key);
    text_placement_info_ptr get_placement_info(double scale_factor, feature_impl const& feature, attributes const& vars, symbol_cache const& sc) const;
    virtual void add_expressions(expression_set & output) const;
    static text_placements_ptr from_xml(xml_node const& xml, fontset_map const& fontsets, bool is_shield);

    text_placements_ptr get_list_placement() { return list_placement_; }

private:
    text_placements_ptr list_placement_;
    symbolizer_base::value_type angle_;
    symbolizer_base::value_type tolerance_;
    symbolizer_base::value_type step_;
    boost::optional<expression_ptr> anchor_key_;

    static constexpr const double default_step = M_PI / 12.0;
    static const box2d<double> empty_box;

    friend class text_placements_info_combined;
};


class text_placement_info_angle: public text_placement_info
{
public:
    text_placement_info_angle(text_placements_angle const* parent,
                              feature_impl const& feature,
                              attributes const& vars,
                              double scale_factor,
                              double angle,
                              double tolerance,
                              double step,
                              box2d<double> const& box,
                              text_placement_info_ptr list_placement_info);
    bool next() const;
    virtual void reset_state();

private:

    struct tolerance_function
    {
        tolerance_function(double step) : step_(step)
        {
        }

        double operator()(double const& linear_position, double const& tolerance) const
        {
            return linear_position * step_;
        }

        const double step_;
    };

    using tolerance_iterator_type = tolerance_iterator<tolerance_function>;

    const text_placement_info_ptr list_placement_info_;
    feature_impl const& feature_;
    attributes const& vars_;
    const double angle_;
    box2d<double> const& box_;

    mutable double dx_;
    mutable double dy_;
    mutable tolerance_iterator_type tolerance_iterator_;

    void get_point(double angle, double &x, double &y) const;
    bool try_next_angle() const;
    double displacement(double const& box_size, symbolizer_base::value_type const& displacement) const;
};

}

#endif
