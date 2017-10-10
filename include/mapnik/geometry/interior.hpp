/*****************************************************************************
 *
 * This file is part of Mapnik (c++ mapping toolkit)
 *
 * Copyright (C) 2017 Artem Pavlenko
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

#ifndef MAPNIK_GEOMETRY_INTERIOR_HPP
#define MAPNIK_GEOMETRY_INTERIOR_HPP

// mapnik
#include <mapnik/geom_util.hpp>
#include <mapnik/geometry/boost_adapters.hpp>

// boost
#include <boost/geometry/algorithms/distance.hpp>
#include <boost/optional.hpp>
#include <boost/utility/in_place_factory.hpp>

// stl
#include <cmath>
#include <vector>
#include <algorithm>

namespace mapnik { namespace geometry {

namespace detail {

using point_type = point<double>;

struct bisector
{
    bisector(point_type const& center, double angle)
        : center(center),
          sin(std::sin(angle)),
          cos(std::cos(angle)),
          enabled(true)
    {
    }

    inline bool intersects(point_type const& p1,
                           point_type const& p2) const
    {
        double d1 = (center.x - p1.x) * sin + (p1.y - center.y) * cos;
        double d2 = (center.x - p2.x) * sin + (p2.y - center.y) * cos;
        return (d1 < 0 && d2 > 0) || (d1 > 0 && d2 < 0);
    }

    inline point_type intersection(point_type const& p1,
                                   point_type const& p2) const
    {
        double denom = (p2.y - p1.y) * cos - (p2.x - p1.x) * sin;
        // if (denom == 0) return { }; // A caller must ensure lines are not parallel
        double c1 = center.x * sin - center.y * cos;
        double c2 = p1.x * p2.y - p1.y * p2.x;
        return { (c1 * (p1.x - p2.x) + cos * c2) / denom,
                 (c1 * (p1.y - p2.y) + sin * c2) / denom };
    }

    inline point_type rotate_back(point_type const& p) const
    {
        return point_type( p.x * cos + p.y * sin,
                          -p.x * sin + p.y * cos);
    }

    point_type center;
    double sin, cos;
    bool enabled;
};

struct intersection
{
    intersection(point_type const& p, double d)
        : position(p), distance(d)
    {
    }

    point_type position;
    double distance; // from center
};

struct intersector
{
    intersector(point_type const& center_, unsigned bisector_count)
        : center(center_),
          sector_angle(M_PI / bisector_count),
          angle_epsilon(std::numeric_limits<double>::epsilon()),
          bisectors(),
          intersections_per_bisector(bisector_count)
    {
        for (unsigned i = 0; i < bisector_count; i++)
        {
            double angle = i * M_PI / bisector_count;
            bisectors.emplace_back(boost::in_place(center, angle));
        }
    }

    template <typename Path>
    void apply(Path & path)
    {
        point<double> p0, p1, move_to;
        unsigned command = SEG_END;
        int sector_p0, sector_p1;

        path.rewind(0);

        while (SEG_END != (command = path.vertex(&p0.x, &p0.y)))
        {
            switch (command)
            {
                case SEG_MOVETO:
                    move_to = p0;
                    process_vertex(p0, sector_p0);
                    break;
                case SEG_CLOSE:
                    p0 = move_to;
                case SEG_LINETO:
                    process_vertex(p0, sector_p0);
                    if (sector_p0 != sector_p1)
                    {
                        process_segment(p0, p1);
                    }
                    break;
            }
            p1 = p0;
            sector_p1 = sector_p0;
        }
    }

    inline void process_vertex(point_type const& vertex, int & sector)
    {
        double angle = 2.0 * M_PI + std::atan2(vertex.y - center.y,
                                               vertex.x - center.x);
        sector = angle / sector_angle;
        if (std::abs(sector * sector_angle - angle) < angle_epsilon)
        {
            std::size_t bisector_index = sector % bisectors.size();
            bisectors[bisector_index] = boost::none;
            intersections_per_bisector[bisector_index].clear();
        }
    }

    inline void process_segment(point_type const& p1,
                                point_type const& p2)
    {
        for (std::size_t bi = 0; bi < bisectors.size(); bi++)
        {
            boost::optional<bisector> const& bisec = bisectors[bi];
            if (bisec && bisec->intersects(p1, p2))
            {
                point_type intersection_point = bisec->intersection(p1, p2);
                point_type relative_intersection(
                    intersection_point.x - bisec->center.x,
                    intersection_point.y - bisec->center.y);
                relative_intersection = bisec->rotate_back(
                    relative_intersection);
                intersections_per_bisector[bi].emplace_back(
                    intersection_point, relative_intersection.x);
            }
        }
    }

    const point_type center;
    const double sector_angle;
    const double angle_epsilon;
    std::vector<boost::optional<bisector>> bisectors;
    std::vector<std::vector<intersection>> intersections_per_bisector;
};


struct placement
{
    placement(point_type const& point_,
              double distance_origin,
              double distance_intersection)
        : position(point_),
          value(std::pow(distance_intersection, 2) /
              (1.0 + std::sqrt(std::abs(distance_origin))))
    {
    }

    bool operator<(placement const& rhs) const
    {
        return value < rhs.value;
    }

    point_type position;
    double value;
};

template <typename Path>
bool interior(Path & path, double & x, double & y, unsigned bisector_count)
{
    // start with the centroid
    if (!label::centroid(path, x, y))
    {
        return false;
    }

    if (bisector_count == 0)
    {
        return true;
    }

    const point_type center(x, y);
    intersector ir(center, bisector_count);
    ir.apply(path);

    multi_point<double> intersection_points;
    for (auto & intersections : ir.intersections_per_bisector)
    {
        for (auto const& i : intersections)
        {
            intersection_points.push_back(i.position);
        }

        std::sort(intersections.begin(), intersections.end(),
            [](intersection const& i1, intersection const& i2) {
                return i1.distance < i2.distance;
            });
    }

    if (intersection_points.size() == 0)
    {
        return true;
    }

    std::vector<placement> placements;

    for (unsigned ipb = 0; ipb < ir.intersections_per_bisector.size(); ipb++)
    {
        auto const& intersections = ir.intersections_per_bisector[ipb];
        for (unsigned i = 1; i < intersections.size(); i += 2)
        {
            intersection const& low = intersections[i - 1];
            intersection const& high = intersections[i];
            point_type position((low.position.x + high.position.x) / 2.0,
                                (low.position.y + high.position.y) / 2.0);
            double distance_origin = (high.distance + low.distance) / 2.0;
            double distance_intersection = boost::geometry::distance(
                position, intersection_points);
            placements.emplace_back(position,
                                    distance_origin,
                                    distance_intersection);
        }
    }

    auto it = std::max_element(placements.begin(), placements.end());
    x = it->position.x;
    y = it->position.y;

    return true;
}

} // namespace detail

template <typename Path>
bool interior(Path & path, double & x, double & y, unsigned bisector_count = 64)
{
    return detail::interior(path, x, y, bisector_count);
}

} }

#endif // MAPNIK_GEOMETRY_INTERIOR_HPP
