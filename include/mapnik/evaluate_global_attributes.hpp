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

#ifndef MAPNIK_EVALUATE_GLOBAL_ATTRIBUTES_HPP
#define MAPNIK_EVALUATE_GLOBAL_ATTRIBUTES_HPP

#include <mapnik/map.hpp>
#include <mapnik/rule.hpp>
#include <mapnik/feature_type_style.hpp>
#include <mapnik/symbolizer.hpp>
#include <mapnik/attribute.hpp>
#include <mapnik/expression_node.hpp>
#include <mapnik/color_factory.hpp>
#include <mapnik/util/noncopyable.hpp>
#include <mapnik/function_call.hpp>
#include <mapnik/util/variant.hpp>

namespace mapnik {

namespace {

template <typename T, typename Attributes>
struct evaluate_expression
{
    using value_type = T;
    using return_type = boost::optional<value_type>;

    explicit evaluate_expression(Attributes const& attributes)
        : attributes_(attributes) {}

    return_type operator() (attribute const&) const
    {
        return boost::none;
        //throw std::runtime_error("can't evaluate feature attributes in this context");
    }

    return_type operator() (global_attribute const& attr) const
    {
        auto itr = attributes_.find(attr.name);
        if (itr != attributes_.end())
        {
            return itr->second;
        }
        return value_type();// throw?
    }

    return_type operator() (geometry_type_attribute const&) const
    {
        return boost::none;
        //throw std::runtime_error("can't evaluate geometry_type attributes in this context");
    }

    return_type operator() (binary_node<tags::logical_and> const & x) const
    {
        auto o1 = util::apply_visitor(*this, x.left);
        auto o2 = util::apply_visitor(*this, x.right);
        if (o1 && o2)
        {
            return o1->to_bool() && o2->to_bool();
        }
        return boost::none;
    }

    return_type operator() (binary_node<tags::logical_or> const & x) const
    {
        auto o1 = util::apply_visitor(*this,x.left);
        auto o2 = util::apply_visitor(*this,x.right);
        if (o1 && o2)
        {
            return o1->to_bool() || o2->to_bool();
        }
        return boost::none;
    }

    template <typename Tag>
    return_type operator() (binary_node<Tag> const& x) const
    {
        auto o1 = util::apply_visitor(*this, x.left);
        auto o2 = util::apply_visitor(*this, x.right);
        if (o1 && o2)
        {
            typename make_op<Tag>::type operation;
            return operation(*o1, *o2);
        }
        return boost::none;
    }

    template <typename Tag>
    return_type operator() (unary_node<Tag> const& x) const
    {
        auto o = util::apply_visitor(*this, x.expr);
        if (o)
        {
            typename make_op<Tag>::type func;
            return func(*o);
        }
        return boost::none;
    }

    return_type operator() (unary_node<tags::logical_not> const& x) const
    {
        auto o = util::apply_visitor(*this, x.expr);
        if (o)
        {
            return ! (o->to_bool());
        }
        return boost::none;
    }

    return_type operator() (regex_match_node const& x) const
    {
        auto o = util::apply_visitor(*this, x.expr);
        if (o)
        {
            return x.apply(*o);
        }
        return boost::none;
    }

    return_type operator() (regex_replace_node const& x) const
    {
        auto o = util::apply_visitor(*this, x.expr);
        if (o)
        {
            return x.apply(*o);
        }
        return boost::none;
    }

    return_type operator() (unary_function_call const& call) const
    {
        auto o = util::apply_visitor(*this, call.arg);
        if (o)
        {
            return call.fun(*o);
        }
        return boost::none;
    }

    return_type operator() (binary_function_call const& call) const
    {
        auto o1 = util::apply_visitor(*this, call.arg1);
        auto o2 = util::apply_visitor(*this, call.arg2);
        if (o1 && o2)
        {
            return call.fun(*o1, *o2);
        }
        return boost::none;
    }

    template <typename ValueType>
    return_type operator() (ValueType const& val) const
    {
        return return_type(value_type(val));
    }

    Attributes const& attributes_;
};

template <typename T>
struct evaluate_expression<T, boost::none_t>
{
    using value_type = T;
    using return_type = boost::optional<value_type>;

    evaluate_expression(boost::none_t) {}

    return_type operator() (attribute const&) const
    {
        return boost::none;
        //throw std::runtime_error("can't evaluate feature attributes in this context");
    }

    return_type operator() (global_attribute const&) const
    {
        return boost::none;
        //throw std::runtime_error("can't evaluate feature attributes in this context");
    }

    return_type operator() (geometry_type_attribute const&) const
    {
        return boost::none;
        //throw std::runtime_error("can't evaluate geometry_type attributes in this context");
    }

    return_type operator() (binary_node<tags::logical_and> const & x) const
    {
        auto o1 = util::apply_visitor(*this, x.left);
        auto o2 = util::apply_visitor(*this, x.right);
        if (o1 && o2)
        {
            return value_type(o1->to_bool() && o2->to_bool());
        }
        return boost::none;
    }

    return_type operator() (binary_node<tags::logical_or> const & x) const
    {
        auto o1 = util::apply_visitor(*this, x.left);
        auto o2 = util::apply_visitor(*this, x.right);
        if (o1 && o2)
        {
            return value_type(o1->to_bool() || o2->to_bool());
        }
        return boost::none;
    }

    template <typename Tag>
    return_type operator() (binary_node<Tag> const& x) const
    {
        auto o1 = util::apply_visitor(*this, x.left);
        auto o2 = util::apply_visitor(*this, x.right);
        if (o1 && o2)
        {
            typename make_op<Tag>::type operation;
            return value_type(operation(*o1, *o2));
        }
        return boost::none;
    }

    template <typename Tag>
    return_type operator() (unary_node<Tag> const& x) const
    {
        auto o = util::apply_visitor(*this, x.expr);
        if (o)
        {
            typename make_op<Tag>::type func;
            return func(*o);
        }
        return boost::none;
    }

    return_type operator() (unary_node<tags::logical_not> const& x) const
    {
        auto o = util::apply_visitor(*this, x.expr);
        if (o)
        {
            return value_type(! o->to_bool());
        }
        return boost::none;
    }

    return_type operator() (regex_match_node const& x) const
    {
        auto o = util::apply_visitor(*this, x.expr);
        if (o)
        {
            return x.apply(*o);
        }
        return boost::none;
    }

    return_type operator() (regex_replace_node const& x) const
    {
        auto o = util::apply_visitor(*this, x.expr);
        if (o)
        {
            return x.apply(*o);
        }
        return boost::none;
    }

    return_type operator() (unary_function_call const& call) const
    {
        auto o = util::apply_visitor(*this, call.arg);
        if (o)
        {
            return call.fun(*o);
        }
        return boost::none;
    }

    return_type operator() (binary_function_call const& call) const
    {
        auto o1 = util::apply_visitor(*this, call.arg1);
        auto o2 = util::apply_visitor(*this, call.arg2);
        if (o1 && o2)
        {
            return call.fun(*o1, *o2);
        }
        return boost::none;
    }

    template <typename ValueType>
    return_type operator() (ValueType const& val) const
    {
        return return_type(value_type(val));
    }
};

struct assign_value
{
    template<typename Attributes>
    static void apply(symbolizer_base::value_type & val, expression_ptr const& expr, Attributes const& attributes, property_types target )
    {

        switch (target)
        {
        case property_types::target_color:
        {
            // evaluate expression as a string then parse as css color
            std::string str = *util::apply_visitor(mapnik::evaluate_expression<mapnik::value,
                                               Attributes>(attributes),*expr).to_string();
            if (boost::optional<color> c = mapnik::parse_color(str))
            {
                val = *c;
            }
            else
            {
                val = color(0,0,0);
            }
            break;
        }
        case property_types::target_double:
        {
            val = *util::apply_visitor(mapnik::evaluate_expression<mapnik::value, Attributes>(attributes),*expr).to_double();
            break;
        }
        case property_types::target_integer:
        {
            val = *util::apply_visitor(mapnik::evaluate_expression<mapnik::value, Attributes>(attributes),*expr).to_int();
            break;
        }
        case property_types::target_bool:
        {
            val = *util::apply_visitor(mapnik::evaluate_expression<mapnik::value, Attributes>(attributes),*expr).to_bool();
            break;
        }
        default: // no-op
            break;
        }
    }
};

}

template <typename T>
std::tuple<T,bool> pre_evaluate_expression (expression_ptr const& expr)
{
    if (boost::optional<T> val = util::apply_visitor(mapnik::evaluate_expression<T, boost::none_t>(boost::none), *expr))
    {
        return std::make_tuple(*val, true);
    }
    return std::make_tuple(T(), false);
}

struct evaluate_global_attributes : util::noncopyable
{
    template <typename Attributes>
    struct evaluator
    {
        evaluator(symbolizer_base::cont_type::value_type & prop, Attributes const& attributes)
            : prop_(prop),
              attributes_(attributes) {}

        void operator() (expression_ptr const& expr) const
        {
            auto const& meta = get_meta(prop_.first);
            assign_value::apply(prop_.second, expr, attributes_, std::get<2>(meta));
        }

        template <typename T>
        void operator() (T const&) const
        {
            // no-op
        }
        symbolizer_base::cont_type::value_type & prop_;
        Attributes const& attributes_;
    };

    template <typename Attributes>
    struct extract_symbolizer
    {
        extract_symbolizer(Attributes const& attributes)
            : attributes_(attributes) {}

        template <typename Symbolizer>
        void operator() (Symbolizer & sym) const
        {
            for (auto & prop : sym.properties)
            {
                util::apply_visitor(evaluator<Attributes>(prop, attributes_), prop.second);
            }
        }
        Attributes const& attributes_;
    };

    template <typename Attributes>
    static void apply(Map & m, Attributes const& attributes)
    {
        for ( auto & val :  m.styles() )
        {
            for (auto & rule : val.second.get_rules_nonconst())
            {
                for (auto & sym : rule)
                {
                    util::apply_visitor(extract_symbolizer<Attributes>(attributes), sym);
                }
            }
        }
    }
};

}

#endif // MAPNIK_EVALUATE_GLOBAL_ATTRIBUTES_HPP
