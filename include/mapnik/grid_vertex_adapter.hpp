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

// agg
#include "agg_rendering_buffer.h"
#include "agg_pixfmt_gray.h"
#include "agg_renderer_base.h"
#include "agg_renderer_scanline.h"
#include "agg_rasterizer_scanline_aa.h"
#include "agg_scanline_bin.h"

namespace mapnik { namespace geometry {

template <typename T>
struct grid_vertex_adapter2
{
    using value_type = typename point<T>::value_type;

    grid_vertex_adapter2(polygon<T> const & geom, T dx, T dy)
        : grid_vertex_adapter2(geom, dx, dy, envelope(geom))
    {
    }

    void rewind(unsigned) const
    {
        x_ = 0;
        y_ = 0;
    }

    unsigned vertex(value_type * x, value_type * y) const
    {
        for (; y_ < count_y_; y_++, x_ = 0)
        {
            for (; x_ < count_x_; x_++)
            {
                *x = start_x_ + dx_ * x_;
                *y = start_y_ + dy_ * y_;
                if (hit_test(geom_, *x, *y, 0))
                {
                    x_++;
                    return mapnik::SEG_MOVETO;
                }
            }
        }
        return mapnik::SEG_END;
    }

    geometry_types type() const
    {
        return geometry_types::MultiPoint;
    }

private:
    grid_vertex_adapter2(polygon<T> const & geom, T dx, T dy, box2d<T> box)
        : geom_(geom),
          dx_(dx),
          dy_(dy),
          count_x_(box.width() / dx_),
          count_y_(box.height() / dy_),
          start_x_(box.minx() + (box.width() - dx_ * count_x_) / 2.0),
          start_y_(box.miny() + (box.height() - dy_ * count_y_) / 2.0),
          x_(0),
          y_(0)
    {
    }

    polygon<T> const & geom_;
    const T dx_, dy_;
    const unsigned count_x_, count_y_;
    const T start_x_, start_y_;
    mutable unsigned x_, y_;
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
        x_ = 0;
        y_ = 0;
    }

    unsigned vertex(value_type * x, value_type * y) const
    {
        for (; y_ < img_.height(); y_++, x_ = 0)
        {
            image_gray8::pixel_type const * row = img_.get_row(y_);
            for (; x_ < img_.width(); x_++)
            {
                if (row[x_])
                {
                    *x = x_;
                    *y = y_;
                    vt_.backward(x, y);
                    x_++;
                    return mapnik::SEG_MOVETO;
                }
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
        : x_(0),
          y_(0),
          img_(box.width() / dx, box.height() / dy),
          vt_(img_.width(), img_.height(), box)
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

    mutable unsigned x_, y_;
    image_gray8 img_;
    view_transform vt_;
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
