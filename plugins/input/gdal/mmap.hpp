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
#include <mapnik/util/singleton.hpp>

#include <boost/interprocess/file_mapping.hpp>
#include <boost/interprocess/mapped_region.hpp>
#include <boost/filesystem.hpp>

#include <gdal_priv.h>

#include <sstream>

#ifdef MAPNIK_THREADSAFE
#include <mutex>
#endif

struct mmapped_vsifile
{
    mmapped_vsifile(std::string const& filepath)
        : mapping(filepath.c_str(), boost::interprocess::read_only),
          region(mapping, boost::interprocess::read_only)
    {
        boost::filesystem::path vsimem_path("/vsimem");
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
        : tiff_file_mapping(filepath),
          overviews_file_mapping()
    {
        std::string ovr_file_name = filepath + ".ovr";
        if (mapnik::util::exists(ovr_file_name))
        {
            try
            {
                overviews_file_mapping = std::make_unique<mmapped_vsifile>(ovr_file_name);
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

class mmapped_tiff_dataset_register
    : public mapnik::singleton<mmapped_tiff_dataset_register, mapnik::CreateStatic>
{
    friend class mapnik::CreateStatic<mmapped_tiff_dataset_register>;

    struct ref_counted_dataset
    {
        ref_counted_dataset(std::string const& filepath)
            : dataset_(filepath), counter_(1)
        {
        }

        mmapped_tiff_dataset dataset_;
        unsigned counter_;
    };

    std::map<std::string, ref_counted_dataset> datasets_;
#ifdef MAPNIK_THREADSAFE
    std::mutex mutex_;
#endif

    mmapped_tiff_dataset_register() = default;
    ~mmapped_tiff_dataset_register() = default;

    mmapped_tiff_dataset_register(mmapped_tiff_dataset_register const &) = delete;

    void decrement(std::string const& filepath)
    {
#ifdef MAPNIK_THREADSAFE
        std::lock_guard<std::mutex> lock(mutex_);
#endif
        auto it = datasets_.find(filepath);
        if (--it->second.counter_ == 0)
        {
            datasets_.erase(it);
        }
    }

public:
    class handle
    {
        friend class mmapped_tiff_dataset_register;

        handle(std::string const& name, mmapped_tiff_dataset & dataset)
            : name(name), mapping(dataset.tiff_file_mapping.name)
        {
        }

        handle(handle const &) = delete;

    public:
        std::string const& name;
        std::string const& mapping;

        ~handle()
        {
            mmapped_tiff_dataset_register::instance().decrement(name);
        }
    };

    friend class handle;

    std::unique_ptr<handle> get(std::string const& filepath)
    {
#ifdef MAPNIK_THREADSAFE
        std::lock_guard<std::mutex> lock(mutex_);
#endif
        auto it = datasets_.find(filepath);
        if (it != datasets_.end())
        {
            ref_counted_dataset & rfd = it->second;
            rfd.counter_++;
            return std::unique_ptr<handle>(new handle(filepath, rfd.dataset_));
        }

        auto ret = datasets_.emplace(filepath, filepath);
        if (ret.second)
        {
            return std::unique_ptr<handle>(new handle(filepath, ret.first->second.dataset_));
        }

        throw std::runtime_error("mmapped_tiff_dataset_register: error allocating new item.");
    }
};

#endif // GDAL_DATASOURCE_MMAP_HPP
