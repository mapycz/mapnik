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

// stl
#include <algorithm>
#include <future>

#include <mapnik/load_map.hpp>
#include <mapnik/util/fs.hpp>

#include "runner.hpp"

namespace visual_tests
{

class renderer_visitor
{
public:
    renderer_visitor(std::string const & name, mapnik::Map const & map, double scale_factor)
        : name_(name), map_(map), scale_factor_(scale_factor)
    {
    }

    template <typename T>
    result operator()(T const & renderer) const
    {
        return renderer.test(name_, map_, scale_factor_);
    }

private:
    std::string const & name_;
    mapnik::Map const & map_;
    double scale_factor_;
};

runner::runner(boost::filesystem::path const & styles_dir,
               boost::filesystem::path const & output_dir,
               boost::filesystem::path const & reference_dir,
               bool overwrite,
               std::size_t jobs)
    : styles_dir_(styles_dir),
      output_dir_(output_dir),
      reference_dir_(reference_dir),
      jobs_(jobs),
      renderers_{ renderer<agg_renderer>(output_dir_, reference_dir_, overwrite),
                  renderer<cairo_renderer>(output_dir_, reference_dir_, overwrite)/*,
                  renderer<grid_renderer>(output_dir_, reference_dir_, overwrite)*/ }
{
}

result_list runner::test_all(report_type & report) const
{
    std::vector<std::string> files = mapnik::util::list_directory(styles_dir_.string());
    return test_parallel(files, report, jobs_);
}

result_list runner::test(std::vector<std::string> const & style_names, report_type & report) const
{
    std::vector<std::string> files(style_names.size());
    std::transform(style_names.begin(), style_names.end(), std::back_inserter(files),
        [&](std::string const & name)
        {
            return (styles_dir_ / (name + ".xml")).string();
        });
    return test_parallel(files, report, jobs_);
}

result_list runner::test_parallel(std::vector<std::string> const & files, report_type & report, std::size_t jobs) const
{
    result_list results;

    if (files.empty())
    {
        return results;
    }

    if (jobs == 0)
    {
        jobs = 1;
    }

    std::size_t chunk_size = files.size() / jobs;

    if (chunk_size == 0)
    {
        chunk_size = files.size();
        jobs = 1;
    }

    std::launch launch(jobs == 1 ? std::launch::deferred : std::launch::async);
    std::vector<std::future<result_list>> futures(jobs);

    for (std::size_t i = 0; i < jobs; i++)
    {
        files_iterator begin(files.begin() + i * chunk_size);
        files_iterator end(files.begin() + (i + 1) * chunk_size);

        // Handle remainder of files.size() / jobs
        if (i == jobs - 1)
        {
            end = files.end();
        }

        futures[i] = std::async(launch, &runner::test_range, this, begin, end, std::ref(report));
    }

    for (auto & f : futures)
    {
        result_list r = f.get();
        std::move(r.begin(), r.end(), std::back_inserter(results));
    }

    return results;
}

result_list runner::test_range(files_iterator begin, files_iterator end, std::reference_wrapper<report_type> report) const
{
    config defaults;
    result_list results;

    for (std::vector<std::string>::const_iterator i = begin; i != end; i++)
    {
        std::string const & file = *i;
        if (file.size() >= 3 && file.substr(file.size() - 3) == "xml")
        {
            try
            {
                result_list r = test_one(file, defaults, report);
                std::move(r.begin(), r.end(), std::back_inserter(results));
            }
            catch (std::exception const& ex)
            {
                result r;
                r.state = STATE_ERROR;
                r.name = file;
                r.error_message = ex.what();
                results.emplace_back(r);
                mapnik::util::apply_visitor(report_visitor(r), report.get());
            }
        }
    }

    return results;
}

result_list runner::test_one(std::string const& style_path, config cfg, report_type & report) const
{
    mapnik::Map m(cfg.sizes.front().width, cfg.sizes.front().height);
    result_list results;

    try
    {
        mapnik::load_map(m, style_path, true);
    }
    catch (std::exception const& ex)
    {
        std::string what = ex.what();
        if (what.find("Could not create datasource") != std::string::npos ||
            what.find("Postgis Plugin: could not connect to server") != std::string::npos)
        {
            return results;
        }
        throw;
    }

    mapnik::parameters const & params = m.get_extra_parameters();

    boost::optional<mapnik::value_integer> status = params.get<mapnik::value_integer>("status", cfg.status);

    if (!*status)
    {
        return results;
    }

    boost::optional<std::string> sizes = params.get<std::string>("sizes");

    if (sizes)
    {
        cfg.sizes.clear();
        parse_map_sizes(*sizes, cfg.sizes);
    }

    boost::filesystem::path p(style_path);
    std::string name(p.stem().string());

    for (map_size const & size : cfg.sizes)
    {
        m.resize(size.width, size.height);

        boost::optional<std::string> bbox_string = params.get<std::string>("bbox");

        if (bbox_string)
        {
            mapnik::box2d<double> bbox;
            bbox.from_string(*bbox_string);
            m.zoom_to_box(bbox);
        }
        else
        {
            m.zoom_all();
        }

        for (double const & scale_factor : cfg.scales)
        {
            for(auto const& ren : renderers_)
            {
                result r = mapnik::util::apply_visitor(renderer_visitor(name, m, scale_factor), ren);
                results.emplace_back(r);
                mapnik::util::apply_visitor(report_visitor(r), report);
            }
        }
    }

    return results;
}

void runner::parse_map_sizes(std::string const & str, std::vector<map_size> & sizes) const
{
    boost::spirit::ascii::space_type space;
    std::string::const_iterator iter = str.begin();
    std::string::const_iterator end = str.end();
    if (!boost::spirit::qi::phrase_parse(iter, end, map_sizes_parser_, space, sizes))
    {
        throw std::runtime_error("Failed to parse list of sizes: '" + str + "'");
    }
}

}

