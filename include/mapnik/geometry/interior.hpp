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

    inline bool intersects(point_type const& p1, point_type const& p2) const
    {
        double d1 = (center.x - p1.x) * sin + (p1.y - center.y) * cos;
        double d2 = (center.x - p2.x) * sin + (p2.y - center.y) * cos;
        return (d1 < 0 && d2 > 0) || (d1 > 0 && d2 < 0);
    }

    inline point_type intersection(point_type const& p1, point_type const& p2) const
    {
        double denom = (p2.y - p1.y) * cos - (p2.x - p1.x) * sin;
        if (denom == 0)
        {
            return { };
        }
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
    double distance; // from origin
};

struct placement
{

    placement(point_type const& point_,
              double distance_origin,
              double distance_intersection)
        : position(point_),
          value(distance_intersection / (1.0 + std::sqrt(std::abs(distance_origin))))
    {
    }

    bool operator<(placement const& rhs) const
    {
        return value < rhs.value;
    }

    point_type position;
    double value;
};

inline void process_vertex(point_type const& vertex,
                           point_type const& center,
                           int & sector,
                           std::vector<bisector> & bisectors)
{
    double angle = 2.0 * M_PI + std::atan2(vertex.y - center.y, vertex.x - center.x);
    const double sector_angle = M_PI / bisectors.size();
    const double angle_epsilon = std::numeric_limits<double>::epsilon();
    sector = angle / sector_angle;
    if (std::abs(sector * sector_angle - angle) < angle_epsilon)
    {
        bisectors[sector % bisectors.size()].enabled = false;
    }
}

inline void process_segment(point_type const& p1,
                            point_type const& p2,
                            point_type const& center,
                            std::vector<bisector> & bisectors,
                            std::vector<std::vector<intersection>> & intersections_per_bisector)
{
    for (std::size_t bi = 0; bi < bisectors.size(); bi++)
    {
        bisector const& bisec = bisectors[bi];
        if (bisec.enabled && bisec.intersects(p1, p2) /* todo */)
        {
            point_type intersection_point = bisec.intersection(p1, p2);
            point_type relative_intersection(intersection_point.x - bisec.center.x,
                                             intersection_point.y - bisec.center.y);
            relative_intersection = bisec.rotate_back(relative_intersection);
            intersections_per_bisector[bi].emplace_back(intersection_point, relative_intersection.x);
        }
    }
}

template <typename Path>
void make_intersections(Path & path,
                        std::vector<bisector> & bisectors,
                        std::vector<std::vector<intersection>> & intersections_per_bisector,
                        point_type const& center)
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
                process_vertex(p0, center, sector_p0, bisectors);
                break;
            case SEG_CLOSE:
                p0 = move_to;
            case SEG_LINETO:
                process_vertex(p0, center, sector_p0, bisectors);
                if (sector_p0 != sector_p1)
                {
                    process_segment(p0, p1, center, bisectors, intersections_per_bisector);
                }
                break;
        }
        p1 = p0;
        sector_p1 = sector_p0;
    }
}

template <typename Path>
bool interior(Path & path, double & x, double & y, unsigned bisector_count)
{
    // start with the centroid
    if (!label::centroid(path, x, y))
    {
        return false;
    }

    const point_type center(x, y);
    std::vector<bisector> bisectors;
    for (unsigned i = 0; i < bisector_count; i++)
    {
        double angle = i * M_PI / bisector_count;
        bisectors.emplace_back(center, angle);
    }

    std::vector<std::vector<intersection>> intersections_per_bisector(bisectors.size());
    make_intersections(path, bisectors, intersections_per_bisector, center);

    multi_point<double> intersection_points;
    for (auto & intersections : intersections_per_bisector)
    {
        for (auto const& i : intersections)
        {
            intersection_points.push_back(i.position);
        }

        // TODO: test
        if ((intersections.size() % 2) != 0)
        {
            throw std::runtime_error("odd intersections");
            return true;
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

    for (unsigned ipb = 0; ipb < intersections_per_bisector.size(); ipb++)
    {
        auto const& intersections = intersections_per_bisector[ipb];
        for (unsigned i = 1; i < intersections.size(); i += 2)
        {
            intersection const& low = intersections[i - 1];
            intersection const& high = intersections[i];
            point_type position((low.position.x + high.position.x) / 2.0,
                                           (low.position.y + high.position.y) / 2.0);
            double distance_origin = (high.distance + low.distance) / 2.0;
            double distance_intersection = std::pow(boost::geometry::distance(position, intersection_points), 2);
            placements.emplace_back(position, distance_origin, distance_intersection);
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
