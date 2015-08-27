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

#ifndef MAPNIK_GRID_ADAPTERS_HPP
#define MAPNIK_GRID_ADAPTERS_HPP

// mapnik
#include <mapnik/vertex.hpp>
#include <mapnik/image.hpp>
#include <mapnik/image_util.hpp>
#include <mapnik/geom_util.hpp>

// agg
#include "agg_rendering_buffer.h"
#include "agg_pixfmt_gray.h"
#include "agg_renderer_base.h"
#include "agg_renderer_scanline.h"
#include "agg_rasterizer_scanline_aa.h"
#include "agg_scanline_bin.h"

namespace mapnik { namespace geometry {

class spiral_iterator
{
public:
    spiral_iterator(int x, int y)
        : size_x(x), size_y(y),
          end_(std::max(x, y) * std::max(x, y)),
          i_(0),
          x_(0), y_(0)
    {
    }

    bool vertex(int * x, int * y)
    {
        while (i_ < end_)
        {
            int xp = x_ + size_x / 2;
            int yp = y_ + size_y / 2;
            if (std::abs(x_) <= std::abs(y_) && (x_ != y_ || x_ >= 0))
            {
                x_ += ((y_ >= 0) ? 1 : -1);
            }
            else
            {
                y_ += ((x_ >= 0) ? -1 : 1);
            }
            ++i_;
            if (xp >= 0 && xp < size_x && yp >= 0 && yp < size_y)
            {
                *x = xp;
                *y = yp;
                return true;
            }
        }
        return false;
    }

    void rewind()
    {
        i_ = 0;
        x_ = 0;
        y_ = 0;
    }

    const int size_x, size_y;

private:
    const int end_;
    int i_;
    int x_, y_;
};

template <typename PathType, typename T>
struct grid_vertex_adapter
{
    using value_type = typename point<T>::value_type;

    grid_vertex_adapter(PathType & path, T dx, T dy)
        : grid_vertex_adapter(path, dx, dy, mapnik::envelope(path))
    {
    }

    void rewind(unsigned) const
    {
        si_.rewind();
        path_.rewind(0);
    }

    unsigned vertex(value_type * x, value_type * y) const
    {
        int x_int, y_int;
        while (si_.vertex(&x_int, &y_int))
        {
            *x = start_x_ + dx_ * x_int;
            *y = start_y_ + dy_ * y_int;
            if (label::hit_test(path_, *x, *y, 0))
            {
                return mapnik::SEG_MOVETO;
            }
        }
        return mapnik::SEG_END;
    }

    geometry_types type() const
    {
        return geometry_types::MultiPoint;
    }

private:
    grid_vertex_adapter(PathType & path, T dx, T dy, box2d<T> box)
        : path_(path),
          dx_(dx),
          dy_(dy),
          si_(box.width() / dx_, box.height() / dy_),
          start_x_(box.minx() + (box.width() - dx_ * si_.size_x) / 2.0),
          start_y_(box.miny() + (box.height() - dy_ * si_.size_y) / 2.0)
    {
    }

    PathType & path_;
    const T dx_, dy_;
    mutable spiral_iterator si_;
    const T start_x_, start_y_;
};

}
}

#endif //MAPNIK_GRID_ADAPTERS_HPP
