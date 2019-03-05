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
#ifndef MAPNIK_TEXT_LINE_POLICY_HPP
#define MAPNIK_TEXT_LINE_POLICY_HPP

#include <mapnik/vertex_cache.hpp>
#include <mapnik/label_placements/max_line_angle_mover.hpp>

namespace mapnik
{

struct text_line_policy
{
    using params_type = label_placement::placement_params;

    text_line_policy(
        vertex_cache & path,
        text_layout_generator const & lg,
        double layout_width,
        params_type const & params)
        : layout_generator_(lg),
          params_(params),
          layout_width_(layout_width),
          minimum_path_length_(lg.get_text_props().minimum_path_length),
          path_(path)
    {
    }

    bool check_size() const
    {
        return !(
            path_.length() < minimum_path_length_ * params_.scale_factor ||
            path_.length() < layout_width_);
    }

    double get_spacing() const
    {
        return spacing_;
    }

    bool next_subpath()
    {
        if (path_.next_subpath())
        {
            spacing_ = init_spacing();
            position_tolerance_ = init_position_tolerance();
            return true;
        }
        return false;
    }

    bool align()
    {
        auto const & root_layout = layout_generator_.layouts_->root_layout();
        horizontal_alignment_e halign = root_layout.horizontal_alignment();

        // halign == H_LEFT -> don't move
        if (halign == H_MIDDLE ||
            halign == H_AUTO)
        {
            if (!path_.forward(spacing_ / 2.0))
            {
                return false;
            }
        }
        else if (halign == H_RIGHT)
        {
            if (!path_.forward(path_.length()))
            {
                return false;
            }
        }
        else if (halign == H_ADJUST)
        {
            if (!path_.forward(path_.length() / 2.0))
            {
                return false;
            }
        }

        double move_dx = root_layout.displacement().x;
        if (move_dx != 0.0)
        {
            vertex_cache::state state = path_.save_state();
            if (!path_.move(move_dx))
            {
                path_.restore_state(state);
            }
        }
        return true;
    }

    bool move(double distance)
    {
        return path_.move(distance);
    }

    bool forward(bool success)
    {
        return path_.forward(spacing_);
    }

    double position_tolerance() const
    {
        return position_tolerance_;
    }

    text_layout_generator const & layout_generator_;
    params_type const & params_;
    const double layout_width_;
    const double minimum_path_length_;
    vertex_cache & path_;
    double spacing_;
    double position_tolerance_;

private:
    double init_spacing() const
    {
        double spacing = layout_generator_.get_text_props().label_spacing;
        int num_labels = 1;
        if (spacing > 0)
        {
            double count_float = path_.length() / (spacing * params_.scale_factor + layout_width_);
            const double epsilon = std::numeric_limits<double>::epsilon() * count_float;
            bool round = std::abs(count_float - std::round(count_float)) < epsilon;
            num_labels = round ? std::round(count_float) : std::floor(count_float);
        }
        if (num_labels <= 0)
        {
            num_labels = 1;
        }
        return path_.length() / num_labels;
    }

    double init_position_tolerance() const
    {
        double tolerance = layout_generator_.get_text_props().label_position_tolerance * params_.scale_factor;
        if (tolerance > 0)
        {
            return tolerance;
        }

        auto const & root_layout = layout_generator_.layouts_->root_layout();
        horizontal_alignment_e halign = root_layout.horizontal_alignment();
        if (halign == H_ADJUST)
        {
            // Let small tolerance by default.
            return 10.0 * params_.scale_factor;
        }

        return spacing_ / 2.0;
    }
};

struct text_max_line_angle_policy : text_line_policy
{
    using params_type = label_placement::placement_params;

    text_max_line_angle_policy(
        vertex_cache & path,
        text_layout_generator const & lg,
        double layout_width,
        params_type const & params,
        double max_angle_diff,
        double max_angle_distance)
        : text_line_policy(path, lg, layout_width, params),
          mover_(path, max_angle_diff, max_angle_distance)
    {
    }

    bool move(double distance)
    {
        if (!text_line_policy::move(distance))
        {
            return false;
        }

        return mover_.move(distance);
    }

    max_line_angle_mover mover_;
};

}//ns mapnik

#endif // MAPNIK_TEXT_LINE_POLICY_HPP
