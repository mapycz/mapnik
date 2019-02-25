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

#ifndef MAPNIK_AGG_HELPERS_HPP
#define MAPNIK_AGG_HELPERS_HPP

// mapnik
#include <mapnik/symbolizer_enumerations.hpp>
#include <mapnik/agg_rasterizer.hpp>

#pragma GCC diagnostic push
#include <mapnik/warning_ignore_agg.hpp>
#include "agg_gamma_functions.h"
#pragma GCC diagnostic pop

namespace mapnik {

template <typename T>
void set_gamma_method(T & ras_ptr, double gamma, gamma_method_enum method)
{
    switch (method)
    {
    case GAMMA_POWER:
        ras_ptr->gamma(agg::gamma_power(gamma));
        break;
    case GAMMA_LINEAR:
        ras_ptr->gamma(agg::gamma_linear(0.0, gamma));
        break;
    case GAMMA_NONE:
        ras_ptr->gamma(agg::gamma_none());
        break;
    case GAMMA_THRESHOLD:
        ras_ptr->gamma(agg::gamma_threshold(gamma));
        break;
    case GAMMA_MULTIPLY:
        ras_ptr->gamma(agg::gamma_multiply(gamma));
        break;
    default:
        ras_ptr->gamma(agg::gamma_power(gamma));
    }
}

template <unsigned Size=rasterizer::aa_scale>
struct gamma_lut
{
    const unsigned mask = Size - 1;

    gamma_lut(double gamma, gamma_method_enum method)
    {
        switch (method)
        {
            case GAMMA_POWER:
                set(agg::gamma_power(gamma));
                break;
            case GAMMA_LINEAR:
                set(agg::gamma_linear(0.0, gamma));
                break;
            case GAMMA_NONE:
                set(agg::gamma_none());
                break;
            case GAMMA_THRESHOLD:
                set(agg::gamma_threshold(gamma));
                break;
            case GAMMA_MULTIPLY:
                set(agg::gamma_multiply(gamma));
                break;
            default:
                set(agg::gamma_power(gamma));
        }
    }

    template <typename GammaFunc>
    void set(GammaFunc const & gamma_func)
    {
        for(int i = 0; i < Size; i++)
        {
            gamma[i] = std::round(gamma_func(static_cast<double>(i) / mask) * mask);
        }
    }

    unsigned operator () (unsigned i) const
    {
        return gamma[i];
    }

    unsigned gamma[Size];
};

struct noop_gamma_func
{
    unsigned operator () (unsigned alpha) const
    {
        return alpha;
    }
};

}

#endif // MAPNIK_AGG_HELPERS_HPP
