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

#ifndef MAPNIK_PARALLELIZE
#define MAPNIK_PARALLELIZE

#include <vector>
#include <future>

namespace mapnik {

namespace util {

template <typename Func>
void parallelize(Func func, unsigned jobs, unsigned work_size)
{
    jobs = std::max(jobs, 1u);

    unsigned chunk_size = work_size / jobs;

    if (chunk_size == 0)
    {
        chunk_size = work_size;
        jobs = 1;
    }

    std::vector<std::future<void>> futures(jobs);
    std::launch launch(jobs == 1 ? std::launch::deferred : std::launch::async);

    for (std::size_t i = 0; i < jobs; i++)
    {
        unsigned chunk_begin = i * chunk_size;
        unsigned chunk_end = (i + 1) * chunk_size;

        // Handle remainder of size / jobs
        if (i == jobs - 1)
        {
            chunk_end = work_size;
        }

        futures[i] = std::async(launch, func, chunk_begin, chunk_end);
    }

    for (auto & f : futures)
    {
        f.get();
    }
}

}

}

#endif // MAPNIK_PARALLELIZE
