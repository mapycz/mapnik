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

#ifndef MAPNIK_LABEL_PLACEMENT_HPP
#define MAPNIK_LABEL_PLACEMENT_HPP

#include <mapnik/label_placements/point.hpp>
#include <mapnik/symbolizer_enumerations.hpp>

namespace mapnik
{

namespace detail
{

template <typename DetectorT, FaceManagerT>
struct label_placement_params
{
    DetectorT & detector;
    FaceManagerT & font_manager;
    feature_impl const & feature;
    attributes const & vars;
    text_placement_info const & placement_info;
    box2d<double> dims;
    double scale_factor;
};

}

using label_placement_params = detail::label_placement_params<label_collision_detector4, face_manager_freetype>;

template <typename Geom, typename Detector>
class text_placement_finder : util::noncopyable
{
public:
    text_placement_finder(label_placement_enum placement_type,
                             Geom &geom,
                             Detector &detector,
                             markers_placement_params const& params)
        : placement_type_(placement_type)
    {
        switch (placement_type)
        {
        default:
        case POINT_PLACEMENT:
            construct(&point_, geom, detector, params);
            break;
        case INTERIOR_PLACEMENT:
            construct(&interior_, geom, detector, params);
            break;
        case LINE_PLACEMENT:
            construct(&line_, geom, detector, params);
            break;
        case VERTEX_PLACEMENT:
            construct(&vertex_, geom, detector, params);
            break;
        }
    }

    ~text_placement_finder()
    {
        switch (placement_type_)
        {
        default:
        case POINT_PLACEMENT:
            destroy(&point_);
            break;
        case INTERIOR_PLACEMENT:
            destroy(&interior_);
            break;
        case LINE_PLACEMENT:
            destroy(&line_);
            break;
        case VERTEX_PLACEMENT:
            destroy(&vertex_);
            break;
        }
    }

    placements_list get(label_placement_params & params)
    {
        switch (placement_type_)
        {
        default:
        case POINT_PLACEMENT:
            return point_.get_point(x, y, angle, ignore_placement);
        case INTERIOR_PLACEMENT:
            return interior_.get_point(x, y, angle, ignore_placement);
        case LINE_PLACEMENT:
            return line_.get_point(x, y, angle, ignore_placement);
        case VERTEX_PLACEMENT:
            return vertex_.get_point(x, y, angle, ignore_placement);
        }
    }

private:
    label_placement_enum const placement_type_;

    union
    {
        markers_point_placement<Geom, Detector> point_;
        markers_line_placement<Geom, Detector> line_;
        markers_interior_placement<Geom, Detector> interior_;
        markers_vertex_first_placement<Geom, Detector> vertex_first_;
    };

    template <typename T>
    static T* construct(T* what, Geom & geom, Detector & detector,
                        markers_placement_params const& params)
    {
        return new(what) T(geom, detector, params);
    }

    template <typename T>
    static void destroy(T* what)
    {
        what->~T();
    }
};

}

#endif // MAPNIK_LABEL_PLACEMENT_HPP
