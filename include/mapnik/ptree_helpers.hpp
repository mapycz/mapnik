/*****************************************************************************
 *
 * This file is part of Mapnik (c++ mapping toolkit)
 *
 * Copyright (C) 2011 Artem Pavlenko
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

#ifndef MAPNIK_PTREE_HELPERS_HPP
#define MAPNIK_PTREE_HELPERS_HPP

// mapnik
#include <mapnik/enumeration.hpp>
#include <mapnik/config_error.hpp>
#include <mapnik/color_factory.hpp>

// boost
#include <boost/property_tree/ptree.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/optional.hpp>

// stl
#include <iostream>
#include <sstream>

namespace mapnik {

template <typename T>
T get(const boost::property_tree::ptree & node, const std::string & name, bool is_attribute,
      const T & default_value);
template <typename T>
T get(const boost::property_tree::ptree & node, const std::string & name, bool is_attribute);
template <typename T>
T get_value(const boost::property_tree::ptree & node, const std::string & name);
template <typename T>
boost::optional<T> get_optional(const boost::property_tree::ptree & node, const std::string & name,
                                bool is_attribute);

template <typename T>
boost::optional<T> get_opt_attr( const boost::property_tree::ptree & node,
                                 const std::string & name)
{
    return get_optional<T>( node, name, true);
}

template <typename T>
boost::optional<T> get_opt_child( const boost::property_tree::ptree & node,
                                  const std::string & name)
{
    return get_optional<T>( node, name, false);
}

template <typename T>
T get_attr( const boost::property_tree::ptree & node, const std::string & name,
            const T & default_value )
{
    return get<T>( node, name, true, default_value);
}

template <typename T>
T get_attr( const boost::property_tree::ptree & node, const std::string & name )
{
    return get<T>( node, name, true );
}

template <typename T>
T get_css( const boost::property_tree::ptree & node, const std::string & name )
{
    return get_value<T>( node, std::string("CSS parameter '") + name + "'");
}

// specialization for color type
template <>
inline color get_css (boost::property_tree::ptree const& node, std::string const& name)
{
    std::string str = get_value<std::string>( node, std::string("CSS parameter '") + name + "'"); ;
    try
    {
        return mapnik::color_factory::from_string(str.c_str());
    }
    catch (...)
    {
        throw config_error(std::string("Failed to parse ") +
                           name + "'. Expected CSS color"  +
                           " but got '" + str + "'");
    }
}


template <typename charT, typename traits>
std::basic_ostream<charT, traits> &
operator << ( std::basic_ostream<charT, traits> & s, const mapnik::color & c )
{
    std::string hex_string( c.to_string() );
    s << hex_string;
    return s;
}

/** Helper for class bool */
class boolean {
public:
    boolean() {}
    boolean(bool b) : b_(b) {}
    boolean(const boolean & b) : b_(b.b_) {}

    operator bool() const
    {
        return b_;
    }
    boolean & operator = (const boolean & other)
    {
        b_ = other.b_;
        return * this;
    }
    boolean & operator = (bool other)
    {
        b_ = other;
        return * this;
    }
private:
    bool b_;
};

/** Special stream input operator for boolean values */
template <typename charT, typename traits>
std::basic_istream<charT, traits> &
operator >> ( std::basic_istream<charT, traits> & s, boolean & b )
{
    std::string word;
    s >> word;
    if ( s )
    {
        if ( word == "true" || word == "yes" || word == "on" ||
             word == "1")
        {
            b = true;
        }
        else if ( word == "false" || word == "no" || word == "off" ||
                  word == "0")
        {
            b = false;
        }
        else
        {
            s.setstate( std::ios::failbit );
        }
    }
    return s;
}

template <typename charT, typename traits>
std::basic_ostream<charT, traits> &
operator << ( std::basic_ostream<charT, traits> & s, const boolean & b )
{
    s << ( b ? "true" : "false" );
    return s;
}

template <typename T>
void set_attr(boost::property_tree::ptree & pt, const std::string & name, const T & v)
{
    pt.put("<xmlattr>." + name, v);
}

/*
  template <>
  void set_attr<bool>(boost::property_tree::ptree & pt, const std::string & name, const bool & v)
  {
  pt.put("<xmlattr>." + name, boolean(v));
  }
*/

class boolean;

template <typename T>
void set_css(boost::property_tree::ptree & pt, const std::string & name, const T & v)
{
    boost::property_tree::ptree & css_node = pt.push_back(
        boost::property_tree::ptree::value_type("CssParameter",
                                                boost::property_tree::ptree()))->second;
    css_node.put("<xmlattr>.name", name );
    css_node.put_value( v );
}

template <typename T>
struct name_trait
{
    static std::string name()
    {
        return "<unknown>";
    }
    // missing name_trait for type ...
    // if you get here you are probably using a new type
    // in the XML file. Just add a name trait for the new
    // type below.
    BOOST_STATIC_ASSERT( sizeof(T) == 0 );
};

#define DEFINE_NAME_TRAIT_WITH_NAME( type, type_name )                  \
    template <>                                                         \
    struct name_trait<type>                                             \
    {                                                                   \
        static std::string name() { return std::string("type ") + type_name; } \
    };

#define DEFINE_NAME_TRAIT( type )               \
    DEFINE_NAME_TRAIT_WITH_NAME( type, #type )

DEFINE_NAME_TRAIT( double )
DEFINE_NAME_TRAIT( float )
DEFINE_NAME_TRAIT( unsigned )
DEFINE_NAME_TRAIT( boolean )
DEFINE_NAME_TRAIT_WITH_NAME( int, "integer" )
DEFINE_NAME_TRAIT_WITH_NAME( std::string, "string" )
DEFINE_NAME_TRAIT_WITH_NAME( color, "color" )

template <typename ENUM, int MAX>
struct name_trait< mapnik::enumeration<ENUM, MAX> >
{
    typedef enumeration<ENUM, MAX> Enum;

    static std::string name()
    {
        std::string value_list("one of [");
        for (unsigned i = 0; i < Enum::MAX; ++i)
        {
            value_list += Enum::get_string( i );
            if ( i + 1 < Enum::MAX ) value_list += ", ";
        }
        value_list += "]";

        return value_list;
    }
};

template <typename T>
T get(const boost::property_tree::ptree & node, const std::string & name, bool is_attribute,
      const T & default_value)
{
    boost::optional<std::string> str;
    if (is_attribute)
    {
        str = node.get_optional<std::string>( std::string("<xmlattr>.") + name );
    }
    else
    {
        str = node.get_optional<std::string>(name+".<xmltext>");
    }

    if ( str ) {
        try
        {
            return boost::lexical_cast<T>( * str );
        }
        catch (const boost::bad_lexical_cast & )
        {
            throw config_error(std::string("Failed to parse ") +
                               (is_attribute ? "attribute" : "child node") + " '" +
                               name + "'. Expected " + name_trait<T>::name() +
                               " but got '" + *str + "'");
        }
    } else {
        return default_value;
    }
}

template <>
inline color get(boost::property_tree::ptree const& node, std::string const& name, bool is_attribute,
                 color const& default_value)
{
    boost::optional<std::string> str;
    if (is_attribute)
    {
        str = node.get_optional<std::string>( std::string("<xmlattr>.") + name );
    }
    else
    {
        str = node.get_optional<std::string>(name+".<xmltext>");
    }

    if ( str )
    {
        try
        {
            return mapnik::color_factory::from_string((*str).c_str());
        }
        catch (...)
        {
            throw config_error(std::string("Failed to parse ") +
                               (is_attribute ? "attribute" : "child node") + " '" +
                               name + "'. Expected " + name_trait<color>::name() +
                               " but got '" + *str + "'");
        }
    }
    else
    {
        return default_value;
    }
}

template <typename T>
T get(const boost::property_tree::ptree & node, const std::string & name, bool is_attribute)
{
    boost::optional<std::string> str;
    if (is_attribute)
    {
        str = node.get_optional<std::string>( std::string("<xmlattr>.") + name);
    }
    else
    {
        str = node.get_optional<std::string>(name+".<xmltext>");
    }

    if ( ! str ) {
        throw config_error(std::string("Required ") +
                           (is_attribute ? "attribute " : "child node ") +
                           "'" + name + "' is missing");
    }
    try
    {
        return boost::lexical_cast<T>( *str );
    }
    catch (const boost::bad_lexical_cast & )
    {
        throw config_error(std::string("Failed to parse ") +
                           (is_attribute ? "attribute" : "child node") + " '" +
                           name + "'. Expected " + name_trait<T>::name() +
                           " but got '" + *str + "'");
    }
}

template <typename T>
T get_value(const boost::property_tree::ptree & node, const std::string & name)
{
    try
    {
        /* NOTE: get_child works as long as there is only one child with that name.
           If this function is used this used this condition must always be satisfied.
        */
        return node.get_child("<xmltext>").get_value<T>();
    }
    catch (boost::property_tree::ptree_bad_path)
    {
        /* If the XML parser did not find any non-empty data element the is no
           <xmltext> node. But we don't want to fail here but simply return a
           default constructed value of the requested type.
        */
        return T();
    }
    catch (...)
    {
        throw config_error(std::string("Failed to parse ") +
                           name + ". Expected " + name_trait<T>::name() +
                           " but got '" + node.data() + "'");
    }
}

template <typename T>
boost::optional<T> get_optional(const boost::property_tree::ptree & node, const std::string & name,
                                bool is_attribute)
{
    boost::optional<std::string> str;
    if (is_attribute)
    {
        str = node.get_optional<std::string>( std::string("<xmlattr>.") + name);
    }
    else
    {
        str = node.get_optional<std::string>(name+".<xmltext>");
    }

    boost::optional<T> result;
    if ( str )
    {
        try
        {
            result = boost::lexical_cast<T>( *str );
        }
        catch (const boost::bad_lexical_cast &)
        {
            throw config_error(std::string("Failed to parse ") +
                               (is_attribute ? "attribute" : "child node") + " '" +
                               name + "'. Expected " + name_trait<T>::name() +
                               " but got '" + *str + "'");
        }
    }

    return result;
}
//

template <>
inline boost::optional<color> get_optional(const boost::property_tree::ptree & node, const std::string & name,
                                           bool is_attribute)
{
    boost::optional<std::string> str;
    if (is_attribute)
    {
        str = node.get_optional<std::string>( std::string("<xmlattr>.") + name);
    }
    else
    {
        str = node.get_optional<std::string>(name+".<xmltext>");
    }

    boost::optional<color> result;
    if ( str )
    {
        try
        {
            //result = boost::lexical_cast<T>( *str );
            result = mapnik::color_factory::from_string((*str).c_str());
        }
        catch (...)
        {
            throw config_error(std::string("Failed to parse ") +
                               (is_attribute ? "attribute" : "child node") + " '" +
                               name + "'. Expected " + name_trait<color>::name() +
                               " but got '" + *str + "'");
        }
    }

    return result;
}

} // end of namespace mapnik

#endif // MAPNIK_PTREE_HELPERS_HPP
