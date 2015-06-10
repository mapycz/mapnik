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

#include <iostream>
#include <string>
#include <mapnik/font_engine_freetype.hpp>
#include <boost/program_options.hpp>

int main(int argc, char** argv)
{
    namespace po = boost::program_options;

    po::options_description desc("visual test runner");
    desc.add_options()
        ("help,h", "produce usage message")
        ("font-dir", po::value<std::vector<std::string>>(), "font search dirs")
        ;

    po::positional_options_description p;
    p.add("font-dir", -1);
    po::variables_map vm;
    po::store(po::command_line_parser(argc, argv).options(desc).positional(p).run(), vm);
    po::notify(vm);

    if (vm.count("help") || !vm.count("font-dir"))
    {
        std::clog << desc << std::endl;
        return 1;
    }

    for (auto const & dir : vm["font-dir"].as<std::vector<std::string>>())
    {
        mapnik::freetype_engine::register_fonts(dir, true);
    }

    for (auto const & face_name : mapnik::freetype_engine::face_names())
    {
        std::clog << face_name << std::endl;
    }

    return 0;
}
