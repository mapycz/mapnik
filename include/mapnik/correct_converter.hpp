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

#ifndef MAPNIK_CORRECT_CONVERTER_HPP
#define MAPNIK_CORRECT_CONVERTER_HPP

#ifdef MAPNIK_CORRECT_CONVERTER

#include <mapnik/vertex.hpp>
#include <mapnik/vertex_adapters.hpp>
#include <mapnik/polygon_vertex_processor.hpp>
#include <mapnik/geometry.hpp>
#include <mapnik/geometry_types.hpp>
#include <mapnik/geometry_correct.hpp>
#include <mapnik/make_unique.hpp>

namespace mapnik
{

template <typename Geometry>
struct correct_converter
{
    using coord_type = double;

    struct polygon_corrector
    {
        polygon_corrector(Geometry & geom)
            : proc_(create_proc(geom)), adapter(proc_.polygon_)
        {
        }

        using proc_type = geometry::polygon_vertex_processor<coord_type>;
        using adapter_type = geometry::polygon_vertex_adapter<coord_type>;

        proc_type create_proc(Geometry & geom) const
        {
            proc_type proc;
            proc.add_path(geom);
            mapnik::geometry::correct(proc.polygon_);
            return proc;
        }

        proc_type proc_;
        adapter_type adapter;
    };

    correct_converter(Geometry & geom)
        : geom_(geom), polygon_corrector_()
    {
        if (geom.type() == geometry::geometry_types::Polygon)
        {
            polygon_corrector_ = std::make_unique<polygon_corrector>(geom);
        }
    }

    unsigned vertex(double * x, double * y)
    {
        if (polygon_corrector_)
        {
            return polygon_corrector_->adapter.vertex(x, y);
        }

        return geom_.vertex(x, y);
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
    std::unique_ptr<polygon_corrector> polygon_corrector_;
};

}

#endif

#endif // MAPNIK_CORRECT_CONVERTER_HPP
