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

#ifndef MAPNIK_RENDERER_COMMON_PROCESS_BUILDING_SYMBOLIZER_HPP
#define MAPNIK_RENDERER_COMMON_PROCESS_BUILDING_SYMBOLIZER_HPP

#include <mapnik/segment.hpp>
#include <mapnik/feature.hpp>
#include <mapnik/vertex_adapters.hpp>
#include <mapnik/path.hpp>
#include <mapnik/util/math.hpp>
#include <mapnik/transform_path_adapter.hpp>

#include <algorithm>
#include <deque>

#pragma GCC diagnostic push
#include <mapnik/warning_ignore_agg.hpp>
#include "agg_conv_contour.h"
#pragma GCC diagnostic pop

namespace mapnik {

struct render_building_symbolizer
{
    using vertex_adapter_type = geometry::polygon_vertex_adapter<double>;
    using transform_path_type = transform_path_adapter<view_transform, vertex_adapter_type>;

    template <typename F1, typename F2, typename F3, typename F4>
    static void apply(feature_impl const& feature,
                      proj_transform const& prj_trans,
                      view_transform const& view_trans,
                      double height,
                      double shadow_angle,
                      double shadow_length,
                      F1 const & face_func,
                      F2 const & frame_func,
                      F3 const & roof_func,
                      F4 const & shadow_func)
    {
        auto const& geom = feature.get_geometry();
        if (geom.is<geometry::polygon<double>>())
        {
            auto const& poly = geom.get<geometry::polygon<double>>();
            vertex_adapter_type va(poly);
            transform_path_type transformed(view_trans, va, prj_trans);
            make_building(transformed, height, shadow_angle, shadow_length,
                face_func, frame_func, roof_func, shadow_func);
        }
        else if (geom.is<geometry::multi_polygon<double>>())
        {
            auto const& multi_poly = geom.get<geometry::multi_polygon<double>>();
            for (auto const& poly : multi_poly)
            {
                vertex_adapter_type va(poly);
                transform_path_type transformed(view_trans, va, prj_trans);
                make_building(transformed, height, shadow_angle, shadow_length,
                    face_func, frame_func, roof_func, shadow_func);
            }
        }
    }

private:
    template <typename GeomVertexAdapter, typename F1, typename F2, typename F3, typename F4>
    static void make_building(GeomVertexAdapter & geom,
            double height,
            double shadow_angle,
            double shadow_length,
            F1 const& face_func,
            F2 const& frame_func,
            F3 const& roof_func,
            F4 const& shadow_func)
    {
        path_type frame(path_type::types::LineString);
        path_type roof(path_type::types::Polygon);
        std::deque<segment_t> face_segments;
        double ring_begin_x, ring_begin_y;
        double x0 = 0;
        double y0 = 0;
        double x,y;
        geom.rewind(0);
        for (unsigned cm = geom.vertex(&x, &y); cm != SEG_END;
             cm = geom.vertex(&x, &y))
        {
            if (cm == SEG_MOVETO)
            {
                frame.move_to(x,y);
                ring_begin_x = x;
                ring_begin_y = y;
            }
            else if (cm == SEG_LINETO)
            {
                frame.line_to(x,y);
                face_segments.emplace_back(x0,y0,x,y);
            }
            else if (cm == SEG_CLOSE)
            {
                frame.close_path();
                if (!face_segments.empty())
                {
                    face_segments.emplace_back(x0, y0, ring_begin_x, ring_begin_y);
                }
            }
            x0 = x;
            y0 = y;
        }

        if (shadow_length > 0)
        {
            shadow_angle = util::normalize_angle(shadow_angle * (M_PI / 180.0));
            for (auto const& seg : face_segments)
            {
                double dx = std::get<2>(seg) - std::get<0>(seg);
                double dy = std::get<3>(seg) - std::get<1>(seg);
                double seg_normal_angle = std::atan2(-dx, -dy);

                double angle_diff = std::abs(seg_normal_angle - shadow_angle);
                double min_angle_diff = std::min((2 * M_PI) - angle_diff, angle_diff);

                if (min_angle_diff <= (M_PI / 2.0))
                {
                    path_type shadow(path_type::types::Polygon);
                    shadow.move_to(std::get<0>(seg), std::get<1>(seg));
                    shadow.line_to(std::get<2>(seg), std::get<3>(seg));
                    shadow.line_to(std::get<2>(seg) + shadow_length * std::cos(shadow_angle),
                                   std::get<3>(seg) - shadow_length * std::sin(shadow_angle));
                    shadow.line_to(std::get<0>(seg) + shadow_length * std::cos(shadow_angle),
                                   std::get<1>(seg) - shadow_length * std::sin(shadow_angle));
                    shadow_func(shadow);
                }
            }
        }

        for (auto const& seg : face_segments)
        {
            path_type faces(path_type::types::Polygon);
            faces.move_to(std::get<0>(seg),std::get<1>(seg));
            faces.line_to(std::get<2>(seg),std::get<3>(seg));
            faces.line_to(std::get<2>(seg),std::get<3>(seg) - height);
            faces.line_to(std::get<0>(seg),std::get<1>(seg) - height);

            face_func(faces);

            frame.move_to(std::get<0>(seg),std::get<1>(seg));
            frame.line_to(std::get<0>(seg),std::get<1>(seg) - height);
        }

        geom.rewind(0);
        for (unsigned cm = geom.vertex(&x, &y); cm != SEG_END;
             cm = geom.vertex(&x, &y))
        {
            if (cm == SEG_MOVETO)
            {
                frame.move_to(x,y - height);
                roof.move_to(x,y - height);
            }
            else if (cm == SEG_LINETO)
            {
                frame.line_to(x,y - height);
                roof.line_to(x,y - height);
            }
            else if (cm == SEG_CLOSE)
            {
                frame.close_path();
                roof.close_path();
            }
        }

        frame_func(frame);
        roof_func(roof);
    }
};

} // namespace mapnik

#endif // MAPNIK_RENDERER_COMMON_PROCESS_BUILDING_SYMBOLIZER_HPP
