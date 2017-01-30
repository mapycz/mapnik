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

#include <mapnik/vertex.hpp>
#include <mapnik/image.hpp>
#include <mapnik/image_util.hpp>
#include <mapnik/geom_util.hpp>
#include <mapnik/view_transform.hpp>

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

struct row_grid_iterator
{
    using coord_type = double;
    using coord2d_type = coord<coord_type, 2>;

    template <typename PathType>
    row_grid_iterator(
        PathType & path,
        box2d<coord_type> const & envelope,
        coord_type dx,
        coord_type dy,
        view_transform const & vt)
      : center_(envelope.width() / 2.0, envelope.height() / 2.0),
        size_x_(std::ceil(vt.width() / dx)),
        size_y_(std::ceil(vt.height() / dy)),
        x_(0), y_(0)
    {
    }

    inline bool vertex(int * x, int * y)
    {
        for (; y_ < size_y_; ++y_, x_ = 0)
        {
            for (; x_ < size_x_; )
            {
                *x = x_;
                *y = y_;
                ++x_;
                return true;
            }
        }

        return false;
    }

    inline void rewind()
    {
        x_ = 0;
        y_ = 0;
    }

    const coord2d_type center_;
    const int size_x_, size_y_;
    int x_, y_;
};

struct spiral_grid_iterator
{
    using coord_type = double;
    using coord2d_type = coord<coord_type, 2>;

    template <typename PathType>
    spiral_grid_iterator(
        PathType & path,
        box2d<coord_type> const & envelope,
        coord_type dx,
        coord_type dy,
        view_transform const & vt)
      : vt_(vt),
        center_(interior(path, envelope)),
        size_x_(std::ceil((vt.width() + std::abs((vt.width() / 2.0) - center_.x) * 2.0) / dx)),
        size_y_(std::ceil((vt.height() + std::abs((vt.height() / 2.0) - center_.y) * 2.0) / dy)),
        si_(size_x_, size_y_)
    {
    }

    template <typename PathType>
    coord2d_type interior(PathType & path, box2d<coord_type> const & envelope) const
    {
        coord2d_type pos;

        if (!label::interior_position(path, pos.x, pos.y))
        {
            pos = envelope.center();
        }

        vt_.forward(&pos.x, &pos.y);

        return pos;
    }

    inline bool vertex(int * x, int * y)
    {
        return si_.vertex(x, y);
    }

    inline void rewind()
    {
        si_.rewind();
    }

    view_transform const & vt_;
    const coord2d_type center_;
    const int size_x_, size_y_;
    spiral_iterator si_;
};

template <typename PathType, typename TransformType>
class transform_path
{
public:
    using coord_type = double;

    transform_path(PathType & path, TransformType const & transform)
        : path_(path), transform_(transform)
    {
    }

    void rewind(unsigned)
    {
        path_.rewind(0);
    }

    unsigned vertex(coord_type * x, coord_type * y)
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

template <typename GridIterator>
struct grid_vertex_adapter
{
    using coord_type = double;
    using coord2d_type = coord<coord_type, 2>;

    template <typename PathType>
    grid_vertex_adapter(PathType & path, coord_type dx, coord_type dy)
        : grid_vertex_adapter(path, dx, dy, mapnik::envelope(path))
    {
    }

    void rewind(unsigned)
    {
        gi_.rewind();
    }

    unsigned vertex(coord_type * x, coord_type * y)
    {
        int pix_x, pix_y;
        while (gi_.vertex(&pix_x, &pix_y))
        {
            pix_x = gi_.center_.x + static_cast<double>(pix_x - gi_.size_x_ / 2) * dx_;
            pix_y = gi_.center_.y + static_cast<double>(pix_y - gi_.size_y_ / 2) * dy_;

            if (pix_x >= 0 && pix_x < img_.width() &&
                pix_y >= 0 && pix_y < img_.height() &&
                get_pixel<image_gray8::pixel_type>(img_, pix_x, pix_y))
            {
                *x = pix_x;
                *y = pix_y;
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

protected:
    template <typename PathType>
    grid_vertex_adapter(
        PathType & path,
        coord_type dx,
        coord_type dy,
        box2d<coord_type> envelope)
        : scale_(get_scale(envelope)),
          dx_(dx * scale_), dy_(dy * scale_),
          img_(create_bitmap(envelope)),
          vt_(img_.width(), img_.height(), envelope),
          gi_(path, envelope, dx_, dy_, vt_)
    {
        transform_path<PathType, view_transform> tp(path, vt_);
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

    double get_scale(box2d<coord_type> const & envelope) const
    {
        coord_type size = std::max(envelope.width(), envelope.height());
        const int max_size = 32768;
        if (size > max_size)
        {
            return max_size / size;
        }
        return 1;
    }

    image_gray8 create_bitmap(box2d<coord_type> const & box) const
    {
        if (!box.valid())
        {
            return image_gray8(0, 0);
        }

        return image_gray8(box.width() * scale_, box.height() * scale_);
    }

    const double scale_;
    const coord_type dx_, dy_;
    image_gray8 img_;
    const view_transform vt_;
    GridIterator gi_;
};

template <typename GridIterator>
struct alternating_grid_vertex_adapter : grid_vertex_adapter<GridIterator>
{
    using coord_type = double;

    template <typename PathType>
    alternating_grid_vertex_adapter(PathType & path, coord_type dx, coord_type dy)
        : grid_vertex_adapter<GridIterator>(path, dx, dy)
    {
    }

    unsigned vertex(coord_type * x, coord_type * y)
    {
        int pix_x, pix_y;
        while (this->gi_.vertex(&pix_x, &pix_y))
        {
            int recentered_x = pix_x - this->gi_.size_x_ / 2;
            int recentered_y = pix_y - this->gi_.size_y_ / 2;

            pix_x = this->gi_.center_.x + static_cast<double>(recentered_x) * this->dx_;
            pix_y = this->gi_.center_.y + static_cast<double>(recentered_y) * this->dy_;

            if (recentered_y % 2 != 0)
            {
                pix_x += this->dx_ / 2.0;
            }

            if (pix_x >= 0 && pix_x < this->img_.width() &&
                pix_y >= 0 && pix_y < this->img_.height() &&
                get_pixel<image_gray8::pixel_type>(this->img_, pix_x, pix_y))
            {
                *x = pix_x;
                *y = pix_y;
                this->vt_.backward(x, y);
                return mapnik::SEG_MOVETO;
            }
        }
        return mapnik::SEG_END;
    }
};

}
}

#endif //MAPNIK_GRID_ADAPTERS_HPP
