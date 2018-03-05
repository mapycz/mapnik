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

#include <sstream>

struct mmapped_vsifile
{
    mmapped_vsifile(std::string const& filepath,
                    std::string const& unique_path)
        : mapping(filepath.c_str(), boost::interprocess::read_only),
          region(mapping, boost::interprocess::read_only)
    {
        boost::filesystem::path vsimem_path("/vsimem");
        vsimem_path /= unique_path.empty() ? unique_id() : unique_path;
        vsimem_path /= filepath;

        virt_file = VSIFileFromMemBuffer(
            vsimem_path.string().c_str(),
            static_cast<GByte*>(region.get_address()),
            region.get_size(),
            false);

        if (virt_file == NULL)
        {
            throw std::runtime_error("Call to VSIFileFromMemBuffer "
                "failed with '" + vsimem_path.string() + "'");
        }

        name = vsimem_path.string();
    }

    std::string unique_id() const
    {
        std::stringstream ss;
        ss << region.get_address();
        return ss.str();
    }

    ~mmapped_vsifile()
    {
        VSIFCloseL(virt_file);
        VSIUnlink(name.c_str());
    }

    boost::interprocess::file_mapping mapping;
    boost::interprocess::mapped_region region;
    VSILFILE * virt_file;
    std::string name;
};

struct mmapped_tiff_dataset
{
    mmapped_tiff_dataset(std::string const& filepath)
        : tiff_file_mapping(filepath, std::string()),
          overviews_file_mapping()
    {
        std::string ovr_file_name = filepath + ".ovr";
        if (mapnik::util::exists(ovr_file_name))
        {
            try
            {
                overviews_file_mapping = std::make_unique<mmapped_vsifile>(
                    ovr_file_name, tiff_file_mapping.unique_id());
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
