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

#include <mapnik/parallel_blur.hpp>

#include "agg_blur.h"
#include "agg_pixfmt_rgba.h"

#include <vector>
#include <future>

namespace mapnik
{

template<class Img>
void process_row(Img & img,
                 unsigned y,
                 unsigned rx,
                 agg::pod_vector<typename Img::color_type> & stack)
{
    using order_type = typename Img::order_type;

    enum order_e
    {
        R = order_type::R,
        G = order_type::G,
        B = order_type::B,
        A = order_type::A
    };

    unsigned x, xp, yp, i;
    unsigned stack_ptr;
    unsigned stack_start;

    const agg::int8u* src_pix_ptr;
          agg::int8u* dst_pix_ptr;
    typename Img::color_type*  stack_pix_ptr;

    unsigned sum_r = 0;
    unsigned sum_g = 0;
    unsigned sum_b = 0;
    unsigned sum_a = 0;
    unsigned sum_in_r = 0;
    unsigned sum_in_g = 0;
    unsigned sum_in_b = 0;
    unsigned sum_in_a = 0;
    unsigned sum_out_r = 0;
    unsigned sum_out_g = 0;
    unsigned sum_out_b = 0;
    unsigned sum_out_a = 0;

    unsigned w   = img.width();
    unsigned h   = img.height();
    unsigned wm  = w - 1;
    unsigned hm  = h - 1;

    unsigned div;
    unsigned mul_sum;
    unsigned shr_sum;

    if (rx > 254)
    {
        rx = 254;
    }

    div = rx * 2 + 1;
    mul_sum = agg::stack_blur_tables<int>::g_stack_blur8_mul[rx];
    shr_sum = agg::stack_blur_tables<int>::g_stack_blur8_shr[rx];
    stack.allocate(div);

    src_pix_ptr = img.pix_ptr(0, y);
    for(i = 0; i <= rx; i++)
    {
        stack_pix_ptr    = &stack[i];
        stack_pix_ptr->r = src_pix_ptr[R];
        stack_pix_ptr->g = src_pix_ptr[G];
        stack_pix_ptr->b = src_pix_ptr[B];
        stack_pix_ptr->a = src_pix_ptr[A];
        sum_r           += src_pix_ptr[R] * (i + 1);
        sum_g           += src_pix_ptr[G] * (i + 1);
        sum_b           += src_pix_ptr[B] * (i + 1);
        sum_a           += src_pix_ptr[A] * (i + 1);
        sum_out_r       += src_pix_ptr[R];
        sum_out_g       += src_pix_ptr[G];
        sum_out_b       += src_pix_ptr[B];
        sum_out_a       += src_pix_ptr[A];
    }
    for(i = 1; i <= rx; i++)
    {
        if(i <= wm) src_pix_ptr += Img::pix_width;
        stack_pix_ptr = &stack[i + rx];
        stack_pix_ptr->r = src_pix_ptr[R];
        stack_pix_ptr->g = src_pix_ptr[G];
        stack_pix_ptr->b = src_pix_ptr[B];
        stack_pix_ptr->a = src_pix_ptr[A];
        sum_r           += src_pix_ptr[R] * (rx + 1 - i);
        sum_g           += src_pix_ptr[G] * (rx + 1 - i);
        sum_b           += src_pix_ptr[B] * (rx + 1 - i);
        sum_a           += src_pix_ptr[A] * (rx + 1 - i);
        sum_in_r        += src_pix_ptr[R];
        sum_in_g        += src_pix_ptr[G];
        sum_in_b        += src_pix_ptr[B];
        sum_in_a        += src_pix_ptr[A];
    }

    stack_ptr = rx;
    xp = rx;
    if(xp > wm) xp = wm;
    src_pix_ptr = img.pix_ptr(xp, y);
    dst_pix_ptr = img.pix_ptr(0, y);
    for(x = 0; x < w; x++)
    {
        dst_pix_ptr[R] = (sum_r * mul_sum) >> shr_sum;
        dst_pix_ptr[G] = (sum_g * mul_sum) >> shr_sum;
        dst_pix_ptr[B] = (sum_b * mul_sum) >> shr_sum;
        dst_pix_ptr[A] = (sum_a * mul_sum) >> shr_sum;
        dst_pix_ptr += Img::pix_width;

        sum_r -= sum_out_r;
        sum_g -= sum_out_g;
        sum_b -= sum_out_b;
        sum_a -= sum_out_a;

        stack_start = stack_ptr + div - rx;
        if(stack_start >= div) stack_start -= div;
        stack_pix_ptr = &stack[stack_start];

        sum_out_r -= stack_pix_ptr->r;
        sum_out_g -= stack_pix_ptr->g;
        sum_out_b -= stack_pix_ptr->b;
        sum_out_a -= stack_pix_ptr->a;

        if(xp < wm)
        {
            src_pix_ptr += Img::pix_width;
            ++xp;
        }

        stack_pix_ptr->r = src_pix_ptr[R];
        stack_pix_ptr->g = src_pix_ptr[G];
        stack_pix_ptr->b = src_pix_ptr[B];
        stack_pix_ptr->a = src_pix_ptr[A];

        sum_in_r += src_pix_ptr[R];
        sum_in_g += src_pix_ptr[G];
        sum_in_b += src_pix_ptr[B];
        sum_in_a += src_pix_ptr[A];
        sum_r    += sum_in_r;
        sum_g    += sum_in_g;
        sum_b    += sum_in_b;
        sum_a    += sum_in_a;

        ++stack_ptr;
        if(stack_ptr >= div) stack_ptr = 0;
        stack_pix_ptr = &stack[stack_ptr];

        sum_out_r += stack_pix_ptr->r;
        sum_out_g += stack_pix_ptr->g;
        sum_out_b += stack_pix_ptr->b;
        sum_out_a += stack_pix_ptr->a;
        sum_in_r  -= stack_pix_ptr->r;
        sum_in_g  -= stack_pix_ptr->g;
        sum_in_b  -= stack_pix_ptr->b;
        sum_in_a  -= stack_pix_ptr->a;
    }
}

template<class Img>
void process_column(Img & img,
                    unsigned x,
                    unsigned ry,
                    agg::pod_vector<typename Img::color_type> & stack)
{
    using order_type = typename Img::order_type;

    enum order_e
    {
        R = order_type::R,
        G = order_type::G,
        B = order_type::B,
        A = order_type::A
    };

    unsigned y, xp, yp, i;
    unsigned stack_ptr;
    unsigned stack_start;

    const agg::int8u* src_pix_ptr;
          agg::int8u* dst_pix_ptr;
    typename Img::color_type*  stack_pix_ptr;

    unsigned sum_r = 0;
    unsigned sum_g = 0;
    unsigned sum_b = 0;
    unsigned sum_a = 0;
    unsigned sum_in_r = 0;
    unsigned sum_in_g = 0;
    unsigned sum_in_b = 0;
    unsigned sum_in_a = 0;
    unsigned sum_out_r = 0;
    unsigned sum_out_g = 0;
    unsigned sum_out_b = 0;
    unsigned sum_out_a = 0;

    unsigned w   = img.width();
    unsigned h   = img.height();
    unsigned wm  = w - 1;
    unsigned hm  = h - 1;

    unsigned div;
    unsigned mul_sum;
    unsigned shr_sum;

    if (ry > 254)
    {
        ry = 254;
    }

    div = ry * 2 + 1;
    mul_sum = agg::stack_blur_tables<int>::g_stack_blur8_mul[ry];
    shr_sum = agg::stack_blur_tables<int>::g_stack_blur8_shr[ry];
    stack.allocate(div);

    int stride = img.stride();
    src_pix_ptr = img.pix_ptr(x, 0);
    for(i = 0; i <= ry; i++)
    {
        stack_pix_ptr    = &stack[i];
        stack_pix_ptr->r = src_pix_ptr[R];
        stack_pix_ptr->g = src_pix_ptr[G];
        stack_pix_ptr->b = src_pix_ptr[B];
        stack_pix_ptr->a = src_pix_ptr[A];
        sum_r           += src_pix_ptr[R] * (i + 1);
        sum_g           += src_pix_ptr[G] * (i + 1);
        sum_b           += src_pix_ptr[B] * (i + 1);
        sum_a           += src_pix_ptr[A] * (i + 1);
        sum_out_r       += src_pix_ptr[R];
        sum_out_g       += src_pix_ptr[G];
        sum_out_b       += src_pix_ptr[B];
        sum_out_a       += src_pix_ptr[A];
    }
    for(i = 1; i <= ry; i++)
    {
        if(i <= hm) src_pix_ptr += stride;
        stack_pix_ptr = &stack[i + ry];
        stack_pix_ptr->r = src_pix_ptr[R];
        stack_pix_ptr->g = src_pix_ptr[G];
        stack_pix_ptr->b = src_pix_ptr[B];
        stack_pix_ptr->a = src_pix_ptr[A];
        sum_r           += src_pix_ptr[R] * (ry + 1 - i);
        sum_g           += src_pix_ptr[G] * (ry + 1 - i);
        sum_b           += src_pix_ptr[B] * (ry + 1 - i);
        sum_a           += src_pix_ptr[A] * (ry + 1 - i);
        sum_in_r        += src_pix_ptr[R];
        sum_in_g        += src_pix_ptr[G];
        sum_in_b        += src_pix_ptr[B];
        sum_in_a        += src_pix_ptr[A];
    }

    stack_ptr = ry;
    yp = ry;
    if(yp > hm) yp = hm;
    src_pix_ptr = img.pix_ptr(x, yp);
    dst_pix_ptr = img.pix_ptr(x, 0);
    for(y = 0; y < h; y++)
    {
        dst_pix_ptr[R] = (sum_r * mul_sum) >> shr_sum;
        dst_pix_ptr[G] = (sum_g * mul_sum) >> shr_sum;
        dst_pix_ptr[B] = (sum_b * mul_sum) >> shr_sum;
        dst_pix_ptr[A] = (sum_a * mul_sum) >> shr_sum;
        dst_pix_ptr += stride;

        sum_r -= sum_out_r;
        sum_g -= sum_out_g;
        sum_b -= sum_out_b;
        sum_a -= sum_out_a;

        stack_start = stack_ptr + div - ry;
        if(stack_start >= div) stack_start -= div;

        stack_pix_ptr = &stack[stack_start];
        sum_out_r -= stack_pix_ptr->r;
        sum_out_g -= stack_pix_ptr->g;
        sum_out_b -= stack_pix_ptr->b;
        sum_out_a -= stack_pix_ptr->a;

        if(yp < hm)
        {
            src_pix_ptr += stride;
            ++yp;
        }

        stack_pix_ptr->r = src_pix_ptr[R];
        stack_pix_ptr->g = src_pix_ptr[G];
        stack_pix_ptr->b = src_pix_ptr[B];
        stack_pix_ptr->a = src_pix_ptr[A];

        sum_in_r += src_pix_ptr[R];
        sum_in_g += src_pix_ptr[G];
        sum_in_b += src_pix_ptr[B];
        sum_in_a += src_pix_ptr[A];
        sum_r    += sum_in_r;
        sum_g    += sum_in_g;
        sum_b    += sum_in_b;
        sum_a    += sum_in_a;

        ++stack_ptr;
        if(stack_ptr >= div) stack_ptr = 0;
        stack_pix_ptr = &stack[stack_ptr];

        sum_out_r += stack_pix_ptr->r;
        sum_out_g += stack_pix_ptr->g;
        sum_out_b += stack_pix_ptr->b;
        sum_out_a += stack_pix_ptr->a;
        sum_in_r  -= stack_pix_ptr->r;
        sum_in_g  -= stack_pix_ptr->g;
        sum_in_b  -= stack_pix_ptr->b;
        sum_in_a  -= stack_pix_ptr->a;
    }
}

struct chunk
{
    unsigned begin;
    unsigned end;
};

template <class Img>
void process_row_chunck(Img & img,
                 chunk const& ch,
                 unsigned rx,
                 agg::pod_vector<typename Img::color_type> & stack)
{
    for (unsigned y = ch.begin; y < ch.end; y++)
    {
        process_row(img, y, rx, stack);
    }
}

template <class Img>
void process_column_chunck(Img & img,
                 chunk const& ch,
                 unsigned ry,
                 agg::pod_vector<typename Img::color_type> & stack)
{
    for (unsigned x = ch.begin; x < ch.end; x++)
    {
        process_column(img, x, ry, stack);
    }
}

MAPNIK_DECL void stack_blur_rgba32_parallel(image_rgba8 & img,
                                            unsigned rx,
                                            unsigned ry,
                                            unsigned jobs)
{
    unsigned w = img.width();
    unsigned h = img.height();

    agg::rendering_buffer buf(img.bytes(), img.width(), img.height(), img.row_size());
    agg::pixfmt_rgba32_pre pixf(buf);

    using color_type = typename agg::pixfmt_rgba32_pre::color_type;
    std::vector<agg::pod_vector<color_type>> stack(jobs);

    if (rx > 0)
    {
        unsigned chunk_size = h / jobs;

        if (chunk_size == 0)
        {
            chunk_size = h;
            jobs = 1;
        }

        std::vector<std::future<void>> futures(jobs);

        for (std::size_t i = 0; i < jobs; i++)
        {
            chunk ch { i * chunk_size, (i + 1) * chunk_size };

            // Handle remainder of h / jobs
            if (i == jobs - 1)
            {
                ch.end = h;
            }

            futures[i] = std::async(std::launch::async,
                                    process_row_chunck<decltype(pixf)>,
                                    std::ref(pixf),
                                    ch,
                                    rx,
                                    std::ref(stack[i]));
        }

        for (auto & f : futures)
        {
            f.get();
        }
    }

    if (ry > 0)
    {
        unsigned chunk_size = w / jobs;

        if (chunk_size == 0)
        {
            chunk_size = w;
            jobs = 1;
        }

        std::vector<std::future<void>> futures(jobs);

        for (std::size_t i = 0; i < jobs; i++)
        {
            chunk ch { i * chunk_size, (i + 1) * chunk_size };

            // Handle remainder of w / jobs
            if (i == jobs - 1)
            {
                ch.end = w;
            }

            futures[i] = std::async(std::launch::async,
                                    process_column_chunck<decltype(pixf)>,
                                    std::ref(pixf),
                                    ch,
                                    ry,
                                    std::ref(stack[i]));
        }

        for (auto & f : futures)
        {
            f.get();
        }
    }

/*
    if (rx > 0)
    {
        for (unsigned y = 0; y < h; y++)
        {
            process_row(img, y, rx, stack[0]);
        }
    }

    if (ry > 0)
    {
        for (unsigned x = 0; x < w; x++)
        {
            process_column(pixf, x, ry, stack[0]);
        }
    }
    */
}

} // end ns
