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

#ifndef MAPNIK_PROPERTIES_UTIL_HPP
#define MAPNIK_PROPERTIES_UTIL_HPP

#include <mapnik/symbolizer_base.hpp>
#include <mapnik/xml_node.hpp>
#include <mapnik/config_error.hpp>

#include <string>

#pragma GCC diagnostic push
#include <mapnik/warning_ignore.hpp>
#include <boost/optional.hpp>
#include <boost/property_tree/ptree_fwd.hpp>
#pragma GCC diagnostic pop

namespace mapnik { namespace detail {

template <typename T, class Enable = void>
struct is_mapnik_enumeration
{
    static constexpr bool value = false;
};

template <typename T>
struct is_mapnik_enumeration<T, typename std::enable_if<std::is_enum<typename T::native_type>::value>::type>
{
    static constexpr bool value = true;
};

template <typename T0, bool is_mapnik_enumeration = false>
struct set_property_from_xml_impl
{
    using target_type = T0;
    template <typename T1>
    static bool apply(T1 & val, char const* name, xml_node const& node)
    {
        bool error = false;
        if (boost::optional<target_type> val_ = node.get_opt_attr<target_type>(name, &error))
        {
            val = *val_;
            return true;
        }
        if (error)
        {
            if (boost::optional<expression_ptr> val_ = node.get_opt_attr<expression_ptr>(name, &error))
            {
                val = *val_;
                return true;
            }
            return false;
        }
        return true;
    }
};


template <>
struct set_property_from_xml_impl<std::string,false>
{
    using target_type = std::string;
    template <typename T1>
    static bool apply(T1 & val, char const* name, xml_node const& node)
    {
        bool error = false;
        if (boost::optional<expression_ptr> val_ = node.get_opt_attr<expression_ptr>(name, &error))
        {
            val = *val_;
            return true;
        }
        if (error)
        {
            if (boost::optional<target_type> val_ = node.get_opt_attr<target_type>(name, &error))
            {
                val = *val_;
                return true;
            }
            return false;
        }
        return true;
    }
};

template <typename T0>
struct set_property_from_xml_impl<T0, true>
{
    using target_enum_type = T0;
    template <typename T1>
    static bool apply(T1 & val, char const* name, xml_node const& node)
    {
        if (boost::optional<std::string> enum_str = node.get_opt_attr<std::string>(name))
        {
            target_enum_type e;
            if (e.from_string(*enum_str))
            {
                val = enumeration_wrapper(e);
                return true;
            }
            if (boost::optional<expression_ptr> expr = node.get_opt_attr<expression_ptr>(name))
            {
                val = *expr;
                return true;
            }
            return false;
        }
        return true;
    }
};

} // namespace detail

template <typename T0, typename T1>
void set_property_from_xml(T1 & val, char const* name, xml_node const& node)
{
    if (!detail::set_property_from_xml_impl<T0, detail::is_mapnik_enumeration<T0>::value>::apply(val, name, node))
    {
        throw config_error("set_property_from_xml '" + std::string(name) + "'");
    }
}

void serialize_property(std::string const& name, symbolizer_base::value_type const& val, boost::property_tree::ptree & node);

} // namespace mapnik

#endif // MAPNIK_PROPERTIES_UTIL_HPP
