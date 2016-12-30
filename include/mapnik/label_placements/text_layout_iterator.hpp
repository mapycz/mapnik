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
#ifndef MAPNIK_LABEL_PLACEMENT_TEXT_LAYOUT_ITERATOR_HPP
#define MAPNIK_LABEL_PLACEMENT_TEXT_LAYOUT_ITERATOR_HPP

#include <mapnik/util/noncopyable.hpp>
#include <mapnik/text/text_layout_generator.hpp>

namespace mapnik { namespace label_placement {

template <typename SubLayout>
class text_layout_iterator : util::noncopyable
{
public:
    using params_type = label_placement::placement_params;

    text_layout_iterator(params_type const & params)
        : sublayout_(params)
    {
    }

    template <typename Geoms>
    bool try_placement(
        text_layout_generator & layout_generator,
        Geoms & geoms)
    {
        while (!geoms.empty() && layout_generator.next())
        {
            for (auto it = geoms.begin(); it != geoms.end(); )
            {
                if (sublayout_.try_placement(layout_generator, *it))
                {
                    it = geoms.erase(it);
                }
                else
                {
                    ++it;
                }
            }

            if (layout_generator.has_placements())
            {
                layout_generator.placements_.emplace_back(
                    std::move(layout_generator.get_placements()));
            }
        }

        return !layout_generator.placements_.empty();
    }

private:
    SubLayout sublayout_;
};

} }

#endif // MAPNIK_LABEL_PLACEMENT_TEXT_LAYOUT_ITERATOR_HPP
