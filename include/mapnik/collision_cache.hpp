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

#ifndef MAPNIK_LABEL_COLLISION_CACHE_HPP
#define MAPNIK_LABEL_COLLISION_CACHE_HPP

// mapnik
#include <mapnik/label_collision_detector.hpp>

#include <map>

#pragma GCC diagnostic push
#include <mapnik/warning_ignore.hpp>
#include <boost/optional.hpp>
#pragma GCC diagnostic pop

namespace mapnik
{

template <typename Detector>
class keyed_collision_cache
{
    std::map<std::string, Detector> cache_;
    Detector default_;

    Detector & get(boost::optional<std::string> const & key)
    {
        if (!key)
        {
            return default_;
        }
        auto it = cache_.find(*key);
        if (it != cache_.end())
        {
            return it->second;
        }
        return cache_.emplace(
            std::piecewise_construct,
            std::forward_as_tuple(*key),
            std::forward_as_tuple(default_.extent())).first->second;
    }

public:
    keyed_collision_cache(box2d<double> const & extent)
        : cache_(), default_(extent)
    {
    }

    bool has_placement(
        box2d<double> const & box,
        boost::optional<std::string> const & key)
    {
        Detector & detector = get(key);
        return detector.has_placement(box);
    }

    bool has_placement(
        box2d<double> const& box,
        double margin,
        boost::optional<std::string> const & key)
    {
        Detector & detector = get(key);
        return detector.has_placement(box, margin);
    }

    bool has_placement(
        box2d<double> const& box,
        double margin,
        mapnik::value_unicode_string const& text,
        double repeat_distance,
        boost::optional<std::string> const & key)
    {
        Detector & detector = get(key);
        return detector.has_placement(box, margin, text, repeat_distance);
    }

    void insert(
        box2d<double> const& box,
        boost::optional<std::string> const & key)
    {
        Detector & detector = get(key);
        detector.insert(box);
    }

    void insert(
        box2d<double> const& box,
        mapnik::value_unicode_string const& text,
        boost::optional<std::string> const & key)
    {
        Detector & detector = get(key);
        detector.insert(box, text);
    }

    void clear()
    {
        default_.clear();
    }

    box2d<double> const& extent() const
    {
        return default_.extent();
    }

    struct iterator_adaper
    {
        Detector & detector_;

        typename Detector::query_iterator begin()
        {
            return detector_.begin();
        }

        typename Detector::query_iterator end()
        {
            return detector_.end();
        }
    };

    iterator_adaper iterate()
    {
        return iterator_adaper{default_};
    }

    iterator_adaper iterate(boost::optional<std::string> const & key)
    {
        if (!key)
        {
            return iterator_adaper{default_};
        }
        auto it = cache_.find(*key);
        if (it == cache_.end())
        {
            MAPNIK_LOG_ERROR(iterate) << "Collision cache '" <<
                *key << "' does not exist. Default cache is used instead.";
            return iterator_adaper{default_};
        }
        return iterator_adaper{it->second};
    }
};

}

#endif // MAPNIK_LABEL_COLLISION_CACHE_HPP
