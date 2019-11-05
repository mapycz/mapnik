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
#pragma once

#include <mapnik/util/noncopyable.hpp>
#include <mapnik/config.hpp>
#include <mapnik/value.hpp>
#include <mapnik/text/face.hpp>

#include <unordered_map>

#pragma GCC diagnostic push
#include <mapnik/warning_ignore.hpp>
#include <unicode/unistr.h>
#include <unicode/uscript.h>
#include <unicode/ubidi.h>
#include <harfbuzz/hb.h>
#pragma GCC diagnostic pop

#include <boost/container_hash/hash.hpp>

namespace mapnik
{

struct shaper_cache_key
{
    UScriptCode script;
    UBiDiDirection dir;
    font_face const & face;
    value_unicode_string text;

    std::size_t hash() const
    {
        std::size_t h = text.hashCode();
        boost::hash_combine(h, script);
        boost::hash_combine(h, dir);
        boost::hash_combine(h, &face);
        return h;
    }

    bool operator==(shaper_cache_key const & other) const
    {
        return text == other.text &&
            script == other.script &&
            dir == other.dir &&
            &face == &other.face;
    }
};

}

namespace std
{
    template<> struct hash<mapnik::shaper_cache_key>
    {
        typedef mapnik::shaper_cache_key argument_type;
        typedef std::size_t result_type;
        result_type operator()(mapnik::shaper_cache_key const& k) const noexcept
        {
            return k.hash();
        }
    };
}

namespace mapnik
{

class shaper_cache : private util::noncopyable
{
    using value_type = std::unique_ptr<hb_buffer_t, decltype(&hb_buffer_destroy)>;

    using cache_type = std::unordered_map<shaper_cache_key, value_type>;
    cache_type cache_;

public:
    hb_buffer_t * find(shaper_cache_key const & key) const
    {
        auto it = cache_.find(key);
        if (it != cache_.end()) {
            return it->second.get();
        }
        return nullptr;
    }

    bool insert(shaper_cache_key && key, value_type && value)
    {
        auto it = cache_.emplace(
            std::piecewise_construct,
            std::forward_as_tuple(std::move(key)),
            std::forward_as_tuple(std::move(value)));
        return it.second;
    }
};

}
