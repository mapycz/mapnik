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
#ifndef MAPNIK_TOLERANCE_ITERATOR_HPP
#define MAPNIK_TOLERANCE_ITERATOR_HPP

//mapnik
#include <mapnik/debug.hpp>

namespace mapnik
{

struct exponential_function
{
    double operator()(double const& linear_position, double const& tolerance) const
    {
        return std::pow(1.3, linear_position) * linear_position /
            (4.0 * tolerance) + linear_position;
    }
};

template <typename Function>
class tolerance_iterator
{
public:
    tolerance_iterator(double label_position_tolerance, Function function)
        : tolerance_(label_position_tolerance),
          linear_position_(1.0),
          value_(0),
          initialized_(false),
          values_tried_(0),
          function_(function)
    {
    }

    tolerance_iterator(double label_position_tolerance)
        : tolerance_iterator(label_position_tolerance, Function())
    {
    }

    ~tolerance_iterator()
    {
        //std::cout << "values tried:" << values_tried_ << "\n";
    }

    double get() const
    {
        return -value_;
    }

    bool next()
    {
        ++values_tried_;
        if (values_tried_ > 255)
        {
            /* This point should not be reached during normal operation. But I can think of
             * cases where very bad spacing and or tolerance values are choosen and the
             * placement finder tries an excessive number of placements.
             * 255 is an arbitrarily chosen limit.
             */
            MAPNIK_LOG_WARN(placement_finder) << "Tried a huge number of placements. Please check "
                                                 "'position-tolerance' and 'spacing' parameters "
                                                 "of your symbolizer.\n";
            return false;
        }
        if (!initialized_)
        {
            initialized_ = true;
            return true; //Always return value 0 as the first value.
        }
        if (value_ == 0)
        {
            value_ = function_(linear_position_, tolerance_);
            return value_ <= tolerance_;
        }
        value_ = -value_;
        if (value_ > 0)
        {
            linear_position_ += 1.0;
            value_ = function_(linear_position_, tolerance_);
        }
        if (value_ > tolerance_)
        {
            return false;
        }
        return true;
    }

    void reset()
    {
        linear_position_ = 1.0;
        value_ = .0;
        initialized_ = false;
        values_tried_ = 0;
    }

private:
    const double tolerance_;
    double linear_position_;
    double value_;
    bool initialized_;
    unsigned values_tried_;
    Function function_;
};

}//ns mapnik

#endif // MAPNIK_TOLERANCE_ITERATOR_HPP
