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

#ifndef MAPNIK_LOG_RENDER_HPP
#define MAPNIK_LOG_RENDER_HPP

#include <mapnik/timer.hpp>
#include <mapnik/debug.hpp>

namespace mapnik {

struct log_render
{
    log_render(std::string const& message)
        : message(message)
    {
    }

    template <typename Timer>
    void operator()(Timer const& t) const
    {
        std::stringstream s;
        s << std::setw(31) << std::right << "| ";
        s << t.to_string();
        s << std::setw(60 - (int)s.tellp()) << std::right << "| ";
        s << message;
        MAPNIK_LOG_PERF() << s.str();
    }

    std::string message;
};

}

#endif // MAPNIK_LOG_RENDER_HPP
