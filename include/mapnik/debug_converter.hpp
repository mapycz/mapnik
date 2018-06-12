/*****************************************************************************
 *
 * This file is part of Mapnik (c++ mapping toolkit)
 *
 * Copyright (C) 2018 Artem Pavlenko
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

#ifndef MAPNIK_DEBUG_CONVERTER_HPP
#define MAPNIK_DEBUG_CONVERTER_HPP

#include <mapnik/vertex.hpp>

#include <iostream>

namespace mapnik
{

template <typename Geometry>
struct debug_converter
{
    debug_converter(Geometry & geom)
        : geom_(geom)
    {}

    unsigned vertex(double * x, double * y)
    {
        unsigned cmd = geom_.vertex(x, y);

        switch (cmd)
        {
            case SEG_MOVETO:
                std::clog << "MOVE ";
                break;
            case SEG_LINETO:
                std::clog << "LINETO ";
                break;
            case SEG_CLOSE:
                std::clog << "CLOSE ";
                break;
            case SEG_END:
                std::clog << "END ";
                break;
        }

        std::clog << *x << "; " << *y << std::endl;

        return cmd;
    }

    void rewind(unsigned)
    {
        geom_.rewind(0);
    }

    unsigned type() const
    {
        return static_cast<unsigned>(geom_.type());
    }

private:
    Geometry & geom_;
};

}

#endif // MAPNIK_DEBUG_CONVERTER_HPP
