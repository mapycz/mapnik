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
#include <mapnik/geometry.hpp>
#include <mapnik/geometry_types.hpp>
#include <mapnik/vertex.hpp>
#include <mapnik/hit_test_filter.hpp>
#include <mapnik/proj_strategy.hpp>
#include <mapnik/view_strategy.hpp>
#include <mapnik/geometry_transform.hpp>
#include <mapnik/image.hpp>
#include <mapnik/image_util.hpp>

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
        : size_x_(x), size_y_(y),
          end_(std::max(x, y) * std::max(x, y)),
          i_(0),
          x_(0), y_(0)
    {
    }

    bool vertex(int * x, int * y)
    {
        while (i_ < end_)
        {
            int xp = x_ + size_x_ / 2;
            int yp = y_ + size_y_ / 2;
            if (std::abs(x_) <= std::abs(y_) && (x_ != y_ || x_ >= 0))
            {
                x_ += ((y_ >= 0) ? 1 : -1);
            }
            else
            {
                y_ += ((x_ >= 0) ? -1 : 1);
            }
            ++i_;
            if (xp >= 0 && xp < size_x_ && yp >= 0 && yp < size_y_)
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

private:
    const int size_x_, size_y_;
    const int end_;
    int i_;
    int x_, y_;
};

template <typename T>
struct grid_vertex_adapter
{
    using value_type = typename point<T>::value_type;

    grid_vertex_adapter(polygon<T> const & geom, T dx, T dy)
        : grid_vertex_adapter(geom, dx, dy, envelope(geom))
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
            if (get_pixel<image_gray8::pixel_type>(img_, x_int, y_int))
            {
                *x = x_int;
                *y = y_int;
                vt_.backward(x, y);
                *x += dx_ / 2.0;
                *y -= dy_ / 2.0;
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
    grid_vertex_adapter(polygon<T> const & geom, T dx, T dy, box2d<T> box)
        : dx_(dx), dy_(dy),
          img_(box.width() / dx, box.height() / dy),
          vt_(img_.width(), img_.height(), box),
          si_(img_.width(), img_.height())
    {
        view_strategy vs(vt_);
        auto geom2 = transform<T>(geom, vs);
        polygon_vertex_adapter<T> va(geom2);
        agg::rasterizer_scanline_aa<> ras;
        ras.add_path(va);

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

    const T dx_, dy_;
    image_gray8 img_;
    const view_transform vt_;
    mutable spiral_iterator si_;
};

template <typename Processor, typename T>
struct grid_vertex_processor
{
    using processor_type = Processor;

    grid_vertex_processor(processor_type & proc, view_strategy const & vs, proj_strategy const & ps, double dx, double dy)
        : proc_(proc), vs_(vs), ps_(ps), dx_(dx), dy_(dy)
    {
    }

    template <typename Geometry>
    void operator() (Geometry const& geom)
    {
    }

    void operator() (polygon<T> const& poly)
    {
        auto geom = transform<double>(transform<double>(poly, ps_), vs_);
        grid_vertex_adapter<T> va(geom, dx_, dy_);
        proc_(va);
    }

    void operator() (multi_polygon<T> const& multi_poly)
    {
        for ( auto const& poly : multi_poly)
        {
            auto geom = transform<double>(transform<double>(poly, ps_), vs_);
            grid_vertex_adapter<T> va(geom, dx_, dy_);
            proc_(va);
        }
    }

    void operator() (geometry_collection<T> const& collection)
    {
        for (auto const& geom : collection)
        {
            operator()(geom);
        }
    }

private:
    processor_type & proc_;
    view_strategy const & vs_;
    proj_strategy const & ps_;
    const T dx_, dy_;
};

}
}

#endif //MAPNIK_GRID_ADAPTERS_HPP
