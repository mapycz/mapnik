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

#ifndef MAPNIK_ALTERNATING_ITERATOR_HPP
#define MAPNIK_ALTERNATING_ITERATOR_HPP

namespace mapnik
{

template <typename Value, template <typename, typename...> class Container, typename... Args>
class alternating_iterator
{
public:
    using iterator_category = std::input_iterator_tag;
    using container_type = Container<Value, Args...>;
    using value_type = Value;

    alternating_iterator(container_type const & cont, std::size_t starting_position)
        : cont_(cont),
          i_(cont.cbegin() + starting_position),
          ri_(cont.crbegin() + (cont.empty() ? 0 : (cont.size() - 1 - starting_position))),
          is_forward_(true)
    {
    }

    alternating_iterator(container_type const & cont)
        : cont_(cont),
          i_(cont.cend()),
          ri_(cont.crend())
    {
    }

    alternating_iterator & operator++()
    {
        do
        {
            if (is_forward_)
            {
                is_forward_ = false;
                if (i_ != cont_.cend())
                {
                    ++i_;
                }
                if (i_ != cont_.cend())
                {
                    break;
                }
            }
            else
            {
                is_forward_ = true;
                if (ri_ != cont_.crend())
                {
                    ++ri_;
                }
                if (ri_ != cont_.crend())
                {
                    break;
                }
            }
        } while (i_ != cont_.cend() || ri_ != cont_.crend());
        return *this;
    }

    value_type const & operator*()
    {
        return is_forward_ ? *ri_ : *i_;
    }

    bool operator!=(const alternating_iterator & rhs)
    {
        return i_ != rhs.i_ || ri_ != rhs.ri_;
    }

private:
    container_type const & cont_;
    typename container_type::const_iterator i_;
    typename container_type::const_reverse_iterator ri_;
    bool is_forward_;
};

}

#endif
