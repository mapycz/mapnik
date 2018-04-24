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

#include <mapnik/label_collision_detector.hpp>
#include <mapnik/debug.hpp>

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
public:
    using detector_type = Detector;

private:
    using cache_type = std::map<std::string, detector_type>;

    cache_type cache_;
    detector_type & default_;

    detector_type & get(boost::optional<std::string> const & key)
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

    template <typename Keys, typename... Args>
    bool detect(Keys const & keys, Args... args)
    {
        if (keys.empty())
        {
            return default_.has_placement(args...);
        }
        for (auto const & key : keys)
        {
            auto it = cache_.find(key);
            if (it != cache_.end())
            {
                detector_type & detector = it->second;
                if (!detector.has_placement(args...))
                {
                    return false;
                }
            }
        }
        return true;
    }

    template <typename Keys, typename... Args>
    void push(Keys const & keys, Args... args)
    {
        if (keys.empty())
        {
            return default_.insert(args...);
        }
        for (auto const & key : keys)
        {
            auto it = cache_.find(key);
            if (it == cache_.end())
            {
                it = cache_.emplace(
                    std::piecewise_construct,
                    std::forward_as_tuple(key),
                    std::forward_as_tuple(default_.extent())).first;
            }

            detector_type & detector = it->second;
            detector.insert(args...);
        }
    }

    detector_type & create_default(box2d<double> const & extent)
    {
        auto it = cache_.emplace(
            std::piecewise_construct,
            std::forward_as_tuple("default"),
            std::forward_as_tuple(extent)).first;
        return it->second;
    }

public:
    keyed_collision_cache(box2d<double> const & extent)
        : cache_(), default_(create_default(extent))
    {
    }

    template <typename Keys>
    bool has_placement(
        box2d<double> const & box,
        Keys const & keys)
    {
        return detect(keys, box);
    }

    template <typename Keys>
    bool has_placement(
        box2d<double> const& box,
        double margin,
        Keys const & keys)
    {
        return detect(keys, box, margin);
    }

    template <typename Keys>
    bool has_placement(
        box2d<double> const& box,
        double margin,
        mapnik::value_unicode_string const& text,
        double repeat_distance,
        Keys const & keys)
    {
        return detect(keys, box, margin, text, repeat_distance);
    }

    template <typename Keys>
    void insert(
        box2d<double> const& box,
        Keys const & keys)
    {
        push(keys, box);
    }

    template <typename Keys>
    void insert(
        box2d<double> const& box,
        mapnik::value_unicode_string const& text,
        Keys const & keys)
    {
        push(keys, box, text);
    }

    void clear()
    {
        default_.clear();
    }

    box2d<double> const& extent() const
    {
        return default_.extent();
    }

    detector_type & detector(std::string const & key)
    {
        auto it = cache_.find(key);
        if (it == cache_.end())
        {
            MAPNIK_LOG_ERROR(detector) << "Collision cache '" <<
                key << "' does not exist. Default cache is used instead.";
            return default_;
        }
        return it->second;
    }

    detector_type const& detector(std::string const & key) const
    {
        auto it = cache_.find(key);
        if (it == cache_.cend())
        {
            MAPNIK_LOG_ERROR(detector) << "Collision cache '" <<
                key << "' does not exist. Default cache is used instead.";
            return default_;
        }
        return it->second;
    }

    std::vector<std::string> keys() const
    {
        std::vector<std::string> ns;
        ns.reserve(cache_.size());
        for (auto const & pair : cache_)
        {
            ns.emplace_back(pair.first);
        }
        return ns;
    }

    detector_type & get_default()
    {
        return default_;
    }
};

std::vector<std::string> parse_collision_detector_keys(
    boost::optional<std::string> const & keys);

}

#endif // MAPNIK_LABEL_COLLISION_CACHE_HPP
