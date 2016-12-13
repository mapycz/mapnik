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

namespace mapnik
{

template <typename LayoutGenerator>
struct text_line_policy
{
    using params_type = label_placement::placement_params;

    text_line_policy(
        vertex_cache & path,
        LayoutGenerator const & lg,
        double layout_width,
        params_type const & params)
        : path_(path),
          layout_generator_(lg),
          params_(params),
          layout_width_(layout_width),
          minimum_path_length_(lg.get_text_props().minimum_path_length),
          spacing_(lg.get_text_props().label_spacing),
          position_tolerance_(
            lg.get_text_props().label_position_tolerance * params.scale_factor)
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
        int num_labels = 1;
        if (spacing_ > 0)
        {
            num_labels = static_cast<int>(std::floor(
                path_.length() / (spacing_ * params_.scale_factor + layout_width_)));
        }
        if (num_labels <= 0)
        {
            num_labels = 1;
        }
        return path_.length() / num_labels;
    }

    bool align()
    {
        double spacing = get_spacing();
        auto const & root_layout = layout_generator_.layouts_->root_layout();
        horizontal_alignment_e halign = root_layout.horizontal_alignment();

        // halign == H_LEFT -> don't move
        if (halign == H_MIDDLE ||
            halign == H_AUTO)
        {
            if (!path_.forward(spacing / 2.0))
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
            spacing = path_.length();
            if (!path_.forward(spacing / 2.0))
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

    vertex_cache & path_;
    LayoutGenerator const & layout_generator_;
    params_type const & params_;
    const double layout_width_;
    const double minimum_path_length_;
    const double spacing_;
    const double position_tolerance_;
};

}//ns mapnik

#endif // MAPNIK_TEXT_LINE_POLICY_HPP
