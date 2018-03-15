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

#ifndef MAPNIK_PARALLEL_SCALE
#define MAPNIK_PARALLEL_SCALE

#include <mapnik/config.hpp>
#include <mapnik/image.hpp>

#pragma GCC diagnostic push
#include <mapnik/warning_ignore_agg.hpp>
#include "agg_image_accessors.h"
#include "agg_pixfmt_rgba.h"
#include "agg_pixfmt_gray.h"
#include "agg_color_rgba.h"
#include "agg_rasterizer_scanline_aa.h"
#include "agg_renderer_scanline.h"
#include "agg_rendering_buffer.h"
#include "agg_scanline_u.h"
#include "agg_span_allocator.h"
#include "agg_span_image_filter_gray.h"
#include "agg_span_image_filter_rgba.h"
#include "agg_span_interpolator_linear.h"
#include "agg_trans_affine.h"
#include "agg_image_filters.h"
#pragma GCC diagnostic pop

#include <vector>
#include <future>
#include <iostream>

namespace mapnik {

namespace detail {

struct chunk
{
    unsigned begin;
    unsigned end;
};

template <typename Img>
void scale(Img const& source,
           Img & dest,
           chunk ch,
           double image_ratio_x,
           double image_ratio_y)
{
    using image_type = Img;
    using pixel_type = typename image_type::pixel_type;
    using pixfmt_pre = typename detail::agg_scaling_traits<image_type>::pixfmt_pre;
    using color_type = typename detail::agg_scaling_traits<image_type>::color_type;
    using img_src_type = typename detail::agg_scaling_traits<image_type>::img_src_type;
    using interpolator_type = typename detail::agg_scaling_traits<image_type>::interpolator_type;
    using renderer_base_pre = agg::renderer_base<pixfmt_pre>;
    constexpr std::size_t pixel_size = sizeof(pixel_type);
    const unsigned y = ch.begin;
    const unsigned height = ch.end - ch.begin;

/*
    unsigned src_row = static_cast<unsigned>(y / image_ratio_y);
    if (src_row >= source.height())
    {
        src_row = source.height() - 1;
    }

    int source_stride = source.width() * pixel_size;
    agg::rendering_buffer rbuf_src(const_cast<unsigned char*>(source.bytes()) + src_row * source_stride,
                                   source.width(),
                                   height / image_ratio_y,
                                   source_stride);
    std::clog << src_row << ", " << y << ", " << (height / image_ratio_y) << std::endl;
    pixfmt_pre pixf_src(rbuf_src);
    img_src_type img_src(pixf_src);
    */

    int source_stride = source.width() * pixel_size;
    agg::rendering_buffer rbuf_src(const_cast<unsigned char*>(source.bytes()),
                                   source.width(),
                                   source.height(),
                                   source_stride);
    pixfmt_pre pixf_src(rbuf_src);
    img_src_type img_src(pixf_src);

/*
    int dest_stride = dest.width() * pixel_size;
    agg::rendering_buffer rbuf_dst(dest.bytes() + y * dest_stride,
                                   dest.width(),
                                   height,
                                   dest_stride);
                                   */

    int dest_stride = dest.width() * pixel_size;
    agg::rendering_buffer rbuf_dst(dest.bytes(),
                                   dest.width(),
                                   dest.height(),
                                   dest_stride);
    pixfmt_pre pixf_dst(rbuf_dst);
    renderer_base_pre rb_dst_pre(pixf_dst);

    agg::trans_affine img_mtx;
    double src_row = y / image_ratio_y;
    img_mtx *= agg::trans_affine_translation(0, 0);
    img_mtx /= agg::trans_affine_scaling(image_ratio_x, image_ratio_y);

    interpolator_type interpolator(img_mtx);

    agg::rasterizer_scanline_aa<> ras;
    agg::scanline_u8 sl;
    agg::span_allocator<color_type> sa;

    double scaled_width = dest.width();
    ras.reset();
    ras.move_to_d(0.0, y);
    ras.line_to_d(scaled_width, y);
    ras.line_to_d(scaled_width, y + height);
    ras.line_to_d(0.0, y + height);

    using span_gen_type = typename detail::agg_scaling_traits<image_type>::span_image_filter_bilinear;
    span_gen_type sg(img_src, interpolator);
    agg::render_scanlines_aa(ras, sl, rb_dst_pre, sa, sg);
}

}

template <typename Img>
MAPNIK_DECL void scale_parallel(Img const& source,
                                Img & dest,
                                unsigned jobs,
                                double image_ratio_x,
                                double image_ratio_y)
{
    jobs = std::max(jobs, 1u);

    unsigned total_size = dest.height();
    unsigned chunk_size = total_size / jobs;

    if (chunk_size == 0)
    {
        chunk_size = total_size;
        jobs = 1;
    }

    std::vector<std::future<void>> futures(jobs);

    for (std::size_t i = 0; i < jobs; i++)
    {
        detail::chunk ch { i * chunk_size, (i + 1) * chunk_size };

        // Handle remainder of size / jobs
        if (i == jobs - 1)
        {
            ch.end = total_size;
        }

        futures[i] = std::async(std::launch::deferred,
                                detail::scale<Img>,
                                std::cref(source),
                                std::ref(dest),
                                ch,
                                image_ratio_x,
                                image_ratio_y);
    }

    for (auto & f : futures)
    {
        f.get();
    }
}

} // end ns mapnik

#endif // MAPNIK_PARALLEL_SCALE
