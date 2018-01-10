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

#ifndef GDAL_DATASOURCE_MMAP_HPP
#define GDAL_DATASOURCE_MMAP_HPP

#include <mapnik/util/fs.hpp>
#include <mapnik/make_unique.hpp>
#include <mapnik/debug.hpp>

#include <boost/interprocess/file_mapping.hpp>
#include <boost/interprocess/mapped_region.hpp>
#include <boost/filesystem.hpp>

#include <gdal_priv.h>

struct mmapped_vsifile
{
    mmapped_vsifile(std::string const& filepath)
        : mapping(filepath.c_str(), boost::interprocess::read_only),
          region(mapping, boost::interprocess::read_only)
    {
        boost::filesystem::path vsimem("/vsimem");
        vsimem /= filepath;

        virt_file = VSIFileFromMemBuffer(
            vsimem.string().c_str(),
            static_cast<GByte*>(region.get_address()),
            region.get_size(),
            false);

        if (virt_file == NULL)
        {
            throw std::runtime_error("Call to VSIFileFromMemBuffer "
                "failed with '" + vsimem.string() + "'");
        }

        name = vsimem.string();
    }

    ~mmapped_vsifile()
    {
        VSIFCloseL(virt_file);
    }

    boost::interprocess::file_mapping mapping;
    boost::interprocess::mapped_region region;
    VSILFILE * virt_file;
    std::string name;
};

struct mmapped_tiff_dataset
{
    mmapped_tiff_dataset(std::string const& filepath)
        : tiff_file_mapping(filepath),
          overviews_file_mapping()
    {
        std::string ovr_file_name = filepath + ".ovr";
        if (mapnik::util::exists(ovr_file_name))
        {
            try
            {
                overviews_file_mapping = std::make_unique<mmapped_vsifile>(
                    ovr_file_name);
            }
            catch (std::exception const& e)
            {
                MAPNIK_LOG_ERROR(mmapped_tiff_dataset) <<
                    "gdal_datasource: Cannot mmap() "
                    " overviews file '" << ovr_file_name <<
                    "': " << e.what();
            }
        }
    }

    mmapped_vsifile tiff_file_mapping;
    std::unique_ptr<mmapped_vsifile> overviews_file_mapping;
};

#endif // GDAL_DATASOURCE_MMAP_HPP
