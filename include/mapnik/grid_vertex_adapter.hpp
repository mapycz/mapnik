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

template <typename PathType, typename T, typename TransformType>
class transform_path
{
public:
    transform_path(PathType & path, TransformType const & transform)
        : path_(path), transform_(transform)
    {
    }

    void rewind(unsigned)
    {
        path_.rewind(0);
    }

    unsigned vertex(T * x, T * y)
    {
        unsigned command = path_.vertex(x, y);
        if (command != mapnik::SEG_END)
        {
            transform_.forward(x, y);
        }
        return command;
    }

private:
    PathType & path_;
    TransformType const & transform_;
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
    }

    unsigned vertex(value_type * x, value_type * y) const
    {
        int x_int, y_int;
        while (si_.vertex(&x_int, &y_int))
        {
            x_int *= dx_;
            y_int *= dy_;
            if (get_pixel<image_gray8::pixel_type>(img_, x_int, y_int))
            {
                *x = x_int;
                *y = y_int;
                vt_.backward(x, y);
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
        : dx_(dx), dy_(dy),
        img_(create_bitmap(box)),
        vt_(img_.width(), img_.height(), box),
        si_(box.width() / dx, box.height() / dy)
    {
        transform_path<PathType, value_type, view_transform> tp(path, vt_);
        tp.rewind(0);
        agg::rasterizer_scanline_aa<> ras;
        ras.add_path(tp);

        agg::rendering_buffer buf(img_.data(), img_.width(), img_.height(), img_.row_size());
        agg::pixfmt_gray8 pixfmt(buf);
        using renderer_base = agg::renderer_base<agg::pixfmt_gray8>;
        using renderer_bin = agg::renderer_scanline_bin_solid<renderer_base>;
        renderer_base rb(pixfmt);
        renderer_bin ren_bin(rb);
        ren_bin.color(agg::gray8(1));
        agg::scanline_bin sl_bin;
        agg::render_scanlines(ras, sl_bin, ren_bin);
    }

    image_gray8 create_bitmap(box2d<T> box)
    {
        if (!box.valid())
        {
            return image_gray8(0, 0);
        }

        try
        {
            return image_gray8(box.width(), box.height());
        }
        catch (std::runtime_error const & e)
        {
            // TODO: Scale down in this case.
            MAPNIK_LOG_ERROR(grid_vertex_adapter) << "grid vertex adapter: Cannot allocate underlaying bitmap. Bbox: "
                << box.to_string() << "; " << e.what();
            return image_gray8(0, 0);
        }
    }

    const T dx_, dy_;
    image_gray8 img_;
    const view_transform vt_;
    mutable spiral_iterator si_;
};

}
}

#endif //MAPNIK_GRID_ADAPTERS_HPP