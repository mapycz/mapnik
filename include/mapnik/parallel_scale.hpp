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
void scale_chunk(Img const& source,
           Img & dest,
           chunk ch,
           scaling_method_e scaling_method,
           double image_ratio_x,
           double image_ratio_y,
           double x_off_f,
           double y_off_f,
           double filter_factor,
           boost::optional<double> nodata_value)
{
    // "the image filters should work namely in the premultiplied color space"
    // http://old.nabble.com/Re:--AGG--Basic-image-transformations-p1110665.html
    // "Yes, you need to use premultiplied images only. Only in this case the simple weighted averaging works correctly in the image fitering."
    // http://permalink.gmane.org/gmane.comp.graphics.agg/3443
    using image_type = Img;
    using pixel_type = typename image_type::pixel_type;
    using pixfmt_pre = typename detail::agg_scaling_traits<image_type>::pixfmt_pre;
    using color_type = typename detail::agg_scaling_traits<image_type>::color_type;
    using img_src_type = typename detail::agg_scaling_traits<image_type>::img_src_type;
    using interpolator_type = typename detail::agg_scaling_traits<image_type>::interpolator_type;
    using renderer_base_pre = agg::renderer_base<pixfmt_pre>;
    constexpr std::size_t pixel_size = sizeof(pixel_type);

    agg::rendering_buffer rbuf_src(const_cast<unsigned char*>(source.bytes()),
                                   source.width(),
                                   source.height(),
                                   source.width() * pixel_size);
    pixfmt_pre pixf_src(rbuf_src);
    img_src_type img_src(pixf_src);

    agg::rendering_buffer rbuf_dst(dest.bytes(),
                                   dest.width(),
                                   dest.height(),
                                   dest.width() * pixel_size);
    pixfmt_pre pixf_dst(rbuf_dst);
    renderer_base_pre rb_dst_pre(pixf_dst);

    agg::trans_affine img_mtx;
    img_mtx *= agg::trans_affine_translation(x_off_f, y_off_f);
    img_mtx /= agg::trans_affine_scaling(image_ratio_x, image_ratio_y);

    interpolator_type interpolator(img_mtx);

    agg::rasterizer_scanline_aa<> ras;
    agg::scanline_u8 sl;
    agg::span_allocator<color_type> sa;

    double scaled_width = dest.width();
    ras.reset();
    ras.move_to_d(0.0, ch.begin);
    ras.line_to_d(scaled_width, ch.begin);
    ras.line_to_d(scaled_width, ch.end);
    ras.line_to_d(0.0, ch.end);

    switch (scaling_method)
    {
        case SCALING_NEAR:
        {
            using span_gen_type = typename detail::agg_scaling_traits<image_type>::span_image_filter;
            span_gen_type sg(img_src, interpolator);
            agg::render_scanlines_aa(ras, sl, rb_dst_pre, sa, sg);
        }
        break;
        case SCALING_BILINEAR_FAST:
        {
            using span_gen_type = typename detail::agg_scaling_traits<image_type>::span_image_filter_bilinear;
            span_gen_type sg(img_src, interpolator);
            agg::render_scanlines_aa(ras, sl, rb_dst_pre, sa, sg);
        }
        break;
        default:
        {
            using span_gen_type = typename detail::agg_scaling_traits<image_type>::span_image_resample_affine;
            agg::image_filter_lut filter;
            detail::set_scaling_method(filter, scaling_method, filter_factor);
            boost::optional<typename span_gen_type::value_type> nodata;
            if (nodata_value)
            {
                nodata = nodata_value;
            }
            span_gen_type sg(img_src, interpolator, filter, nodata);
            agg::render_scanlines_aa(ras, sl, rb_dst_pre, sa, sg);
        }
    }
}

}

template <typename Img>
MAPNIK_DECL void scale_parallel(Img const& source,
                                Img & dest,
                                unsigned jobs,
                                scaling_method_e scaling_method,
                                double image_ratio_x,
                                double image_ratio_y,
                                double x_off_f,
                                double y_off_f,
                                double filter_factor,
                                boost::optional<double> nodata_value)
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
    std::launch launch(jobs == 1 ? std::launch::deferred : std::launch::async);

    for (std::size_t i = 0; i < jobs; i++)
    {
        detail::chunk ch { i * chunk_size, (i + 1) * chunk_size };

        // Handle remainder of size / jobs
        if (i == jobs - 1)
        {
            ch.end = total_size;
        }

        futures[i] = std::async(launch,
                                detail::scale_chunk<Img>,
                                std::cref(source),
                                std::ref(dest),
                                ch,
                                scaling_method,
                                image_ratio_x,
                                image_ratio_y,
                                x_off_f,
                                y_off_f,
                                filter_factor,
                                nodata_value);
    }

    for (auto & f : futures)
    {
        f.get();
    }
}

} // end ns mapnik

#endif // MAPNIK_PARALLEL_SCALE
