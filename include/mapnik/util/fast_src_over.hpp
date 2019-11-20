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

#pragma once

namespace mapnik { namespace util {

union rgba_pixel
{
    std::uint32_t value;
    std::uint8_t channels[4];
};

inline void simple_src_over(
    rgba_pixel * __restrict__ dst,
    const rgba_pixel * __restrict__ src,
    std::size_t length)
{
    for (unsigned p = 0; p < length; ++p)
    {
        std::uint8_t * __restrict__ c1 = dst->channels;
        const std::uint8_t * __restrict__ c2 = src->channels;
        std::uint8_t a = 255 - src->channels[3];
        for (unsigned i = 0; i < 4; ++i)
        {
            c1[i] = static_cast<std::uint8_t>(
                (static_cast<unsigned short>(c2[i]) +
                 ((static_cast<unsigned short>(c1[i]) * a + 255) >> 8)));
        }
        ++dst;
        ++src;
    }
}

inline void simple_multiply(
    rgba_pixel * __restrict__ dst,
    const rgba_pixel * __restrict__ src,
    std::size_t length)
{
    for (unsigned p = 0; p < length; ++p)
    {
        std::uint8_t * __restrict__ c1 = dst->channels;
        const std::uint8_t * __restrict__ c2 = src->channels;
        std::uint8_t a = 255 - src->channels[3];
        for (unsigned i = 0; i < 4; ++i)
        {
            c1[i] = static_cast<std::uint8_t>(
                (static_cast<unsigned short>(c2[i]) +
                 ((static_cast<unsigned short>(c1[i]) * a + 255) >> 8)));
        }
        ++dst;
        ++src;
    }
}
} }
