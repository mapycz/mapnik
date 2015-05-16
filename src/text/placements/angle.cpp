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
#include <mapnik/text/placements/angle.hpp>
#include <mapnik/xml_node.hpp>
#include <mapnik/symbolizer.hpp>
#include <mapnik/text/placements/list.hpp>
#include <mapnik/text/placements/dummy.hpp>

namespace
{

using namespace mapnik;

double normalize_angle(double angle)
{
    angle = std::fmod(angle, M_PI * 2.0);
    if (angle < 0)
    {
        angle += M_PI * 2.0;
    }
    return angle;
}

double extract_angle(feature_impl const& feature,
                     attributes const& vars,
                     symbolizer_base::value_type const& angle_expression)
{
    double angle = util::apply_visitor(extract_value<value_double>(feature, vars), angle_expression);
    return normalize_angle(angle * (M_PI / 180.0));
}

symbolizer_base::value_type get_opt(xml_node const& xml, std::string const& name)
{
    boost::optional<expression_ptr> expression = xml.get_opt_attr<expression_ptr>(name);
    return expression ? *expression : static_cast<symbolizer_base::value_type>(.0);
}

}

namespace mapnik
{

//text_placements_angle class

const box2d<double> text_placements_angle::empty_box;

text_placements_angle::text_placements_angle(text_placements_ptr list_placement,
                                             symbolizer_base::value_type const& angle,
                                             symbolizer_base::value_type const& tolerance,
                                             symbolizer_base::value_type const& step,
                                             boost::optional<expression_ptr> const& anchor_key)
    : list_placement_(list_placement),
      angle_(angle),
      tolerance_(tolerance),
      step_(step),
      anchor_key_(anchor_key)
{
}

text_placement_info_ptr text_placements_angle::get_placement_info(double scale_factor,
                                                                  feature_impl const& feature,
                                                                  attributes const& vars,
                                                                  symbol_cache const& sc) const
{
    text_placement_info_ptr list_placement_info = list_placement_->get_placement_info(scale_factor, feature, vars, sc);

    double angle = extract_angle(feature, vars, angle_);
    double tolerance = extract_angle(feature, vars, tolerance_);
    double step = extract_angle(feature, vars, step_);

    if (std::abs(step) < std::numeric_limits<double>::epsilon())
    {
        step = default_step;
    }

    if (anchor_key_)
    {
        std::string anchor_key = util::apply_visitor(extract_value<std::string>(feature, vars),
            static_cast<symbolizer_base::value_type>(*anchor_key_));

        if (sc.contains(anchor_key))
        {
            symbol_cache::symbol const& sym = sc.get(anchor_key);
            return std::make_shared<text_placement_info_angle>(this,
                feature, vars, scale_factor, angle, tolerance, step, sym.box, list_placement_info);
        }
        else
        {
            return std::make_shared<text_placement_info_dummy>(this, scale_factor, 1);
        }
    }

    return std::make_shared<text_placement_info_angle>(this,
        feature, vars, scale_factor, angle, tolerance, step, empty_box, list_placement_info);
}

void text_placements_angle::add_expressions(expression_set & output) const
{
    list_placement_->add_expressions(output);
    if (is_expression(angle_)) output.insert(util::get<expression_ptr>(angle_));
    if (is_expression(tolerance_)) output.insert(util::get<expression_ptr>(tolerance_));
    if (is_expression(step_)) output.insert(util::get<expression_ptr>(step_));
    if (anchor_key_) output.insert(*anchor_key_);
    text_placements::add_expressions(output);
}

text_placements_ptr text_placements_angle::from_xml(xml_node const& xml, fontset_map const& fontsets, bool is_shield)
{
    symbolizer_base::value_type angle = get_opt(xml, "angle");
    symbolizer_base::value_type tolerance = get_opt(xml, "tolerance");
    symbolizer_base::value_type step = get_opt(xml, "step");
    boost::optional<expression_ptr> anchor_key = xml.get_opt_attr<expression_ptr>("anchor-key");

    text_placements_ptr list_placement_ptr = text_placements_list::from_xml(xml, fontsets, is_shield);

    text_placements_ptr ptr = std::make_shared<text_placements_angle>(list_placement_ptr, angle, tolerance, step, anchor_key);
    ptr->defaults.from_xml(xml, fontsets, is_shield);
    return ptr;
}

//text_placement_info_angle class

text_placement_info_angle::text_placement_info_angle(text_placements_angle const* parent,
                                                     feature_impl const& feature,
                                                     attributes const& vars,
                                                     double scale_factor,
                                                     double angle,
                                                     double tolerance,
                                                     double step,
                                                     box2d<double> const& box,
                                                     text_placement_info_ptr list_placement_info)
    : text_placement_info(parent, scale_factor),
      list_placement_info_(list_placement_info),
      feature_(feature),
      vars_(vars),
      angle_(angle),
      box_(box),
      dx_(displacement(box.width(), properties.layout_defaults.dx)),
      dy_(displacement(box.height(), properties.layout_defaults.dy)),
      tolerance_iterator_(tolerance, 0, tolerance_function(step))
{
    list_placement_info_->next();
}

void text_placement_info_angle::reset_state()
{
    list_placement_info_->reset_state();
    list_placement_info_->next();
    tolerance_iterator_.reset();
}

bool text_placement_info_angle::try_next_angle() const
{
    if (tolerance_iterator_.next())
    {
        double angle = normalize_angle(angle_ + tolerance_iterator_.get());
        double dx, dy;
        get_point(angle, dx, dy);
        properties.layout_defaults.dx = dx;
        properties.layout_defaults.dy = dy;
        return true;
    }
    return false;
}

bool text_placement_info_angle::next() const
{
    if (try_next_angle())
    {
        return true;
    }
    else if (list_placement_info_->next())
    {
        properties = list_placement_info_->properties;
        dx_ = displacement(box_.width(), properties.layout_defaults.dx);
        dy_ = displacement(box_.height(), properties.layout_defaults.dy);
        tolerance_iterator_.reset();
        try_next_angle();
        return true;
    }
    return false;
}

double text_placement_info_angle::displacement(double const& box_size, symbolizer_base::value_type const& displacement) const
{
    double d = util::apply_visitor(extract_value<double>(feature_, vars_), displacement);
    return box_size / 2.0 + d + .5;
}

void text_placement_info_angle::get_point(double angle, double &x, double &y) const
{
    double corner = std::atan(dx_ / dy_);
    if ((angle >= 0 && angle <= corner) || (angle >= 2 * M_PI - corner && angle <= 2 * M_PI))
    {
        x = -dy_ * std::tan(angle);
        y = -dy_;
    }
    else if (angle >= corner && angle <= M_PI - corner)
    {
        x = -dx_;
        y = dx_ * std::tan(angle - M_PI_2);
    }
    else if (angle >= M_PI + corner && angle <= 2 * M_PI - corner)
    {
        x = dx_;
        y = -dx_ * std::tan(angle - M_PI_2 * 3);
    }
    else
    {
        x = dy_ * std::tan(angle - M_PI);
        y = dy_;
    }
}

}
