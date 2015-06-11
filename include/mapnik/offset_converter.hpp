/*****************************************************************************
 *
 * This file is part of Mapnik (c++ mapping toolkit)
 *
 * Copyright (C) 2014 Artem Pavlenko
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

#ifndef MAPNIK_OFFSET_CONVERTER_HPP
#define MAPNIK_OFFSET_CONVERTER_HPP

#ifdef MAPNIK_LOG
#include <mapnik/debug.hpp>
#endif
#include <mapnik/global.hpp>
#include <mapnik/config.hpp>
#include <mapnik/vertex.hpp>
#include <mapnik/vertex_cache.hpp>

// stl
#include <cmath>
#include <vector>
#include <cstddef>

namespace mapnik
{

template <typename Geometry>
struct offset_converter
{
    using size_type = std::size_t;

    offset_converter(Geometry & geom)
        : geom_(geom)
        , offset_(0.0)
        , threshold_(5.0)
        , half_turn_segments_(16)
        , status_(initial)
        , pre_first_(vertex2d::no_init)
        , pre_(vertex2d::no_init)
        , cur_(vertex2d::no_init)
        {}

    enum status
    {
        initial,
        process
    };

    unsigned type() const
    {
        return static_cast<unsigned>(geom_.type());
    }

    double get_offset() const
    {
        return offset_;
    }

    double get_threshold() const
    {
        return threshold_;
    }

    void set_offset(double value)
    {
        if (offset_ != value)
        {
            offset_ = value;
            reset();
        }
    }

    void set_threshold(double value)
    {
        threshold_ = value;
        // no need to reset(), since threshold doesn't affect
        // offset vertices' computation, it only controls how
        // far will we be looking for self-intersections
    }

    unsigned vertex(double * x, double * y)
    {
        if (offset_ == 0.0)
        {
            return geom_.vertex(x, y);
        }

        if (status_ == initial)
        {
            init_vertices();
        }

        if (pos_ >= vertices_.size())
        {
            return SEG_END;
        }

        pre_ = (pos_ ? cur_ : pre_first_);
        cur_ = vertices_.at(pos_++);

        if (pos_ == vertices_.size())
        {
            return output_vertex(x, y);
        }

        double const check_dist = offset_ * threshold_;
        double const check_dist2 = check_dist * check_dist;
        double t = 1.0;
        double vt, ut;

        for (size_t i = pos_; i+1 < vertices_.size(); ++i)
        {
            //break; // uncomment this to see all the curls

            vertex2d const& u0 = vertices_[i];
            vertex2d const& u1 = vertices_[i+1];
            double const dx = u0.x - cur_.x;
            double const dy = u0.y - cur_.y;

            if (dx*dx + dy*dy > check_dist2)
            {
                break;
            }

            if (!intersection(pre_, cur_, &vt, u0, u1, &ut))
            {
                continue;
            }

            if (vt < 0.0 || vt > t || ut < 0.0 || ut > 1.0)
            {
                continue;
            }

            t = vt;
            pos_ = i+1;
        }

        cur_.x = pre_.x + t * (cur_.x - pre_.x);
        cur_.y = pre_.y + t * (cur_.y - pre_.y);
        return output_vertex(x, y);
    }

    void reset()
    {
        geom_.rewind(0);
        vertices_.clear();
        status_ = initial;
        pos_ = 0;
    }

    void rewind(unsigned)
    {
        pos_ = 0;
    }

private:

    static double explement_reflex_angle(double angle)
    {
        if (angle > M_PI)
        {
            return angle - 2 * M_PI;
        }
        else if (angle < -M_PI)
        {
            return angle + 2 * M_PI;
        }
        else
        {
            return angle;
        }
    }

    static bool intersection(vertex2d const& u1, vertex2d const& u2, double* ut,
                             vertex2d const& v1, vertex2d const& v2, double* vt)
    {
        double const dx = v1.x - u1.x;
        double const dy = v1.y - u1.y;
        double const ux = u2.x - u1.x;
        double const uy = u2.y - u1.y;
        double const vx = v2.x - v1.x;
        double const vy = v2.y - v1.y;

        // the first line is not vertical
        if (ux < -1e-6 || ux > 1e-6)
        {
            double const up = ux * dy - dx * uy;
            double const dn = vx * uy - ux * vy;

            if (dn > -1e-6 && dn < 1e-6)
            {
                return false; // they are parallel
            }

            *vt = up / dn;
            *ut = (*vt * vx + dx) / ux;
            return true;
        }

        // the first line is not horizontal
        if (uy < -1e-6 || uy > 1e-6)
        {
            double const up = uy * dx - dy * ux;
            double const dn = vy * ux - uy * vx;

            if (dn > -1e-6 && dn < 1e-6)
            {
                return false; // they are parallel
            }

            *vt = up / dn;
            *ut = (*vt * vy + dy) / uy;
            return true;
        }

        // the first line is too short
        return false;
    }

    /**
     *  @brief  Translate (vx, vy) by rotated (dx, dy).
     */
    static void displace(vertex2d & v, double dx, double dy, double a)
    {
        v.x += dx * std::cos(a) + dy * std::sin(a);
        v.y += dx * std::sin(a) - dy * std::cos(a);
    }

    /**
     *  @brief  Translate (vx, vy) by rotated (0, -offset).
     */
    void displace(vertex2d & v, double a) const
    {
        v.x += offset_ * std::sin(a);
        v.y -= offset_ * std::cos(a);
    }

    /**
     *  @brief  (vx, vy) := (ux, uy) + rotated (0, -offset)
     */
    void displace(vertex2d & v, vertex2d const& u, double a) const
    {
        v.x = u.x + offset_ * std::sin(a);
        v.y = u.y - offset_ * std::cos(a);
        v.cmd = u.cmd;
    }

    void displace2(vertex2d & v, double a, double b) const
    {
        double sa = offset_ * std::sin(a);
        double ca = offset_ * std::cos(a);
        double h = std::tan(0.5 * (b - a));
        if (h > 1.5)
        {
            h = 1.5;
        }
        else if (h < -1.5)
        {
            h = -1.5;
        }
        v.x = v.x + sa + h * ca;
        v.y = v.y - ca + h * sa;
    }

    status init_vertices()
    {
        if (status_ != initial) // already initialized
        {
            return status_;
        }
        vertex2d v0(vertex2d::no_init);
        vertex2d v1(vertex2d::no_init);
        vertex2d v2(vertex2d::no_init);
        vertex2d w(vertex2d::no_init);
        vertex2d start_v2(vertex2d::no_init);
        std::vector<vertex2d> points;
        std::vector<vertex2d> close_points;
        bool is_polygon = false;
        int cpt = 0;
        v0.cmd = geom_.vertex(&v0.x, &v0.y);
        v1.x = v0.x;
        v1.y = v0.y;
        v1.cmd = v0.cmd;
        // PUSH INITIAL
        points.push_back(vertex2d(v0.x, v0.y, v0.cmd));
        while ((v0.cmd = geom_.vertex(&v0.x, &v0.y)) != SEG_END)
        {
            points.push_back(vertex2d(v0.x, v0.y, v0.cmd));
            if (v0.cmd == SEG_CLOSE)
            {
                is_polygon = true;
                close_points.push_back(vertex2d(v1.x, v1.y, v1.cmd));
            }
            v1.x = v0.x;
            v1.y = v0.y;
            v1.cmd = v0.cmd;
        }
        // Push SEG_END
        points.push_back(vertex2d(v0.x, v0.y, v0.cmd));
        std::size_t i = 0;
        v1 = points[i++];
        v2 = points[i++];
        v0.cmd = v1.cmd;
        v0.x = v1.x;
        v0.y = v1.y;

        if (v2.cmd == SEG_END) // not enough vertices in source
        {
            return status_ = process;
        }

        double angle_a = 0;
        if (is_polygon) 
        {
            double x = v1.x - close_points[cpt].x;
            double y = v1.y - close_points[cpt].y;
            cpt++;
            x = std::abs(x) < std::numeric_limits<double>::epsilon() ? 0 : x; 
            y = std::abs(y) < std::numeric_limits<double>::epsilon() ? 0 : y; 
            angle_a = std::atan2(y, x);
        }
        double angle_b = std::atan2((v2.y - v1.y), (v2.x - v1.x));
        double joint_angle;

        if (!is_polygon)
        {
            // first vertex
            displace(v1, angle_b);
            push_vertex(v1);
        }
        else
        {
            joint_angle = explement_reflex_angle(angle_b - angle_a);

            double half_turns = half_turn_segments_ * std::fabs(joint_angle);
            int bulge_steps = 0;

            if (offset_ < 0.0)
            {
                if (joint_angle > 0.0)
                {
                    joint_angle = joint_angle - 2 * M_PI;
                }
                else
                {
                    bulge_steps = 1 + static_cast<int>(std::floor(half_turns / M_PI));
                }
            }
            else
            {
                if (joint_angle < 0.0)
                {
                    joint_angle = joint_angle + 2 * M_PI;
                }
                else
                {
                    bulge_steps = 1 + static_cast<int>(std::floor(half_turns / M_PI));
                }
            }
            if (bulge_steps == 0)
            {
                displace2(v1, angle_a, angle_b);
                push_vertex(v1);
            }
            else
            {
                displace(v1, angle_b);
                push_vertex(v1);
            }
        }

        // Sometimes when the first segment is too short, it causes ugly
        // curls at the beginning of the line. To avoid this, we make up
        // a fake vertex two offset-lengths before the first, and expect
        // intersection detection smoothes it out.
        if (!is_polygon)
        {
            pre_first_ = v1;
            displace(pre_first_, -2 * std::fabs(offset_), 0, angle_b);
            start_ = pre_first_;
        }
        else
        {
            pre_first_ = v0;
            start_ = pre_first_;
        }
        start_v2.x = v2.x;
        start_v2.y = v2.y;
        bool continue_loop = true;
        while (i < points.size())
        {
            v1 = v2;
            v2 = points[i++];
            if (v1.cmd == SEG_MOVETO)
            {
                if (is_polygon)
                {
                    v1.x = start_.x;
                    v1.y = start_.y;
                    if (cpt < close_points.size())
                    {
                        double x = v1.x - close_points[cpt].x;
                        double y = v1.y - close_points[cpt].y;
                        x = std::abs(x) < std::numeric_limits<double>::epsilon() ? 0.0 : x; 
                        y = std::abs(y) < std::numeric_limits<double>::epsilon() ? 0.0 : y; 
                        angle_b = std::atan2(y,x);
                        cpt++;
                    }
                }
                start_v2.x = v2.x;
                start_v2.y = v2.y;
            }
            if (v2.cmd == SEG_MOVETO)
            {
                start_.x = v2.x;
                start_.y = v2.y;
                v2.x = start_v2.x;
                v2.y = start_v2.y;
            }
            else if (v2.cmd == SEG_END)
            {
                if (!is_polygon) break;
                continue_loop = false;
                v2.x = start_v2.x;
                v2.y = start_v2.y;
            }
            else if (v2.cmd == SEG_CLOSE)
            {
                v2.x = start_.x;
                v2.y = start_.y;
            }
            angle_a = angle_b;
            angle_b = std::atan2((v2.y - v1.y), (v2.x - v1.x));
            joint_angle = explement_reflex_angle(angle_b - angle_a);

            double half_turns = half_turn_segments_ * std::fabs(joint_angle);
            int bulge_steps = 0;

            if (offset_ < 0.0)
            {
                if (joint_angle > 0.0)
                {
                    joint_angle = joint_angle - 2 * M_PI;
                }
                else
                {
                    bulge_steps = 1 + static_cast<int>(std::floor(half_turns / M_PI));
                }
            }
            else
            {
                if (joint_angle < 0.0)
                {
                    joint_angle = joint_angle + 2 * M_PI;
                }
                else
                {
                    bulge_steps = 1 + static_cast<int>(std::floor(half_turns / M_PI));
                }
            }

            #ifdef MAPNIK_LOG
            if (bulge_steps == 0)
            {
                // inside turn (sharp/obtuse angle)
                MAPNIK_LOG_DEBUG(ctrans) << "offset_converter:"
                    << " Sharp joint [<< inside turn " << int(joint_angle*180/M_PI)
                    << " degrees >>]";
            }
            else
            {
                // outside turn (reflex angle)
                MAPNIK_LOG_DEBUG(ctrans) << "offset_converter:"
                    << " Bulge joint >)) outside turn " << int(joint_angle*180/M_PI)
                    << " degrees ((< with " << bulge_steps << " segments";
            }
            #endif
            if (v1.cmd == SEG_MOVETO)
            {
                if (bulge_steps == 0)
                {
                    displace2(v1, angle_a, angle_b);
                    push_vertex(v1);
                }
                else
                {
                    displace(v1, angle_b);
                    push_vertex(v1);
                }
            }
            else
            {
                if (bulge_steps == 0)
                {
                    displace2(v1, angle_a, angle_b);
                    push_vertex(v1);
                }
                else
                {
                    displace(w, v1, angle_a);
                    w.cmd = SEG_LINETO;
                    push_vertex(w);

                    for (int s = 0; ++s < bulge_steps;)
                    {
                        displace(w, v1, angle_a + (joint_angle * s) / bulge_steps);
                        w.cmd = SEG_LINETO;
                        push_vertex(w);
                    }
                    displace(v1, angle_b);
                    push_vertex(v1);
                }
            }
        }

        // last vertex
        if (!is_polygon)
        {
            displace(v1, angle_b);
            push_vertex(v1);
        }
        // initialization finished
        return status_ = process;
    }

    unsigned output_vertex(double* px, double* py)
    {
        if (cur_.cmd == SEG_CLOSE) *px = *py = 0.0;
        else
        {
            *px = cur_.x;
            *py = cur_.y;
        }
        return cur_.cmd;
    }

    void push_vertex(vertex2d const& v)
    {
        vertices_.push_back(v);
    }

    Geometry &              geom_;
    double                  offset_;
    double                  threshold_;
    unsigned                half_turn_segments_;
    status                  status_;
    size_t                  pos_;
    std::vector<vertex2d>   vertices_;
    vertex2d                start_;
    vertex2d                pre_first_;
    vertex2d                pre_;
    vertex2d                cur_;
};

}

#endif // MAPNIK_OFFSET_CONVERTER_HPP
