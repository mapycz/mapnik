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

#ifndef MAPNIK_TIMER_HPP
#define MAPNIK_TIMER_HPP

// stl
#include <cstdlib>
#include <string>
#include <sstream>
#include <iomanip>
#include <ctime>

#ifdef _WINDOWS
#define NOMINMAX
#include <windows.h>
#else
#include <sys/time.h> // for gettimeofday() on unix
#include <sys/resource.h>
#endif


namespace mapnik {


// Try to return the time now
inline double time_now()
{
#ifdef _WINDOWS
    LARGE_INTEGER t, f;
    QueryPerformanceCounter(&t);
    QueryPerformanceFrequency(&f);
    return double(t.QuadPart) / double(f.QuadPart);
#else
    struct timeval t;
    struct timezone tzp;
    gettimeofday(&t, &tzp);
    return t.tv_sec + t.tv_usec * 1e-6;
#endif
}


// Measure times in both wall clock time and CPU times. Results are returned in milliseconds.
class timer
{
public:
    timer()
    {
        restart();
    }

    ~timer()
    {
    }

    void restart()
    {
        stopped_ = false;
        wall_clock_start_ = time_now();
        cpu_start_ = clock();
    }

    void stop()
    {
        stopped_ = true;
        cpu_end_ = clock();
        wall_clock_end_ = time_now();
    }

    bool stopped() const
    {
        return stopped_;
    }

    double cpu_elapsed() const
    {
        // return elapsed CPU time in ms
        return ((double) (cpu_end_ - cpu_start_)) / CLOCKS_PER_SEC * 1000.0;
    }

    double wall_clock_elapsed() const
    {
        // return elapsed wall clock time in ms
        return (wall_clock_end_ - wall_clock_start_) * 1000.0;
    }

    std::string to_string() const
    {
        std::ostringstream s;
        s.precision(2);
        s << std::fixed;
        s << wall_clock_elapsed() << "ms (cpu " << cpu_elapsed() << "ms)";
        return s.str();
    }

private:
    double wall_clock_start_, wall_clock_end_;
    clock_t cpu_start_, cpu_end_;
    bool stopped_;
};

template <class Action>
class timer_with_action
{
public:
    timer_with_action(Action const& action)
        : action_(action),
          timer_()
    {}

    ~timer_with_action()
    {
        if (!timer_.stopped())
        {
            timer_.stop();
            try
            {
                action_(timer_);
            }
            catch (...) {} // eat any exceptions
        }
    }

protected:
    Action const& action_;
    timer timer_;
};

//  A progress_timer behaves like a timer except that the destructor displays
//  an elapsed time message at an appropriate place in an appropriate form.
class progress_timer : public timer
{
public:
    progress_timer(std::ostream & os, std::string const& base_message)
        : os_(os),
          base_message_(base_message),
          timer_(*this)
    {}

    void operator()(timer const& t) const
    {
        std::ostringstream s;
        s << t.to_string();
        s << std::setw(30 - (int)s.tellp()) << std::right << "|";
        s << " " << base_message_ << std::endl;
        os_ << s.str();
    }

protected:
    std::ostream & os_;
    std::string base_message_;
    timer_with_action<progress_timer> timer_;
};

}

#endif // MAPNIK_TIMER_HPP
