#include "catch.hpp"

// mapnik
#include <mapnik/vertex.hpp>
#include <mapnik/extend_converter.hpp>

// stl
#include <iostream>
#include <vector>
#include <tuple>

namespace offset_test {

struct fake_path
{
    using coord_type = std::tuple<double, double, unsigned>;
    using cont_type = std::vector<coord_type>;
    cont_type vertices_;
    cont_type::iterator itr_;

    fake_path(std::initializer_list<double> l)
        : fake_path(l.begin(), l.size())
    {
    }

    fake_path(std::vector<double> const &v, bool make_invalid = false)
        : fake_path(v.begin(), v.size(), make_invalid)
    {
    }

    template <typename Itr>
    fake_path(Itr itr, size_t sz, bool make_invalid = false)
    {
        size_t num_coords = sz >> 1;
        vertices_.reserve(num_coords + (make_invalid ? 1 : 0));
        if (make_invalid)
        {
            vertices_.push_back(std::make_tuple(0,0,mapnik::SEG_END));
        }

        for (size_t i = 0; i < num_coords; ++i)
        {
            double x = *itr++;
            double y = *itr++;
            unsigned cmd = (i == 0) ? mapnik::SEG_MOVETO : mapnik::SEG_LINETO;
            vertices_.push_back(std::make_tuple(x, y, cmd));
        }
        itr_ = vertices_.begin();
    }

    unsigned vertex(double *x, double *y)
    {
        if (itr_ == vertices_.end())
        {
            return mapnik::SEG_END;
        }
        *x = std::get<0>(*itr_);
        *y = std::get<1>(*itr_);
        unsigned cmd = std::get<2>(*itr_);
        ++itr_;
        return cmd;
    }

    void rewind(unsigned)
    {
        itr_ = vertices_.begin();
    }
};

TEST_CASE("extend converter") {

SECTION("empty") {
    try
    {
        fake_path path = {};
        mapnik::extend_converter<fake_path> c(path, 1000);
        double x, y;
        REQUIRE(c.vertex(&x, &y) == mapnik::SEG_END);
    }
    catch (std::exception const& ex)
    {
        std::cerr << ex.what() << "\n";
        REQUIRE(false);
    }
}

SECTION("one point") {
    try
    {
        fake_path path = { 0, 0 };
        mapnik::extend_converter<fake_path> c(path, 1000);
        double x, y;
        REQUIRE(c.vertex(&x, &y) == mapnik::SEG_MOVETO);
        REQUIRE(x == 0);
        REQUIRE(y == 0);
        REQUIRE(c.vertex(&x, &y) == mapnik::SEG_END);
        REQUIRE(c.vertex(&x, &y) == mapnik::SEG_END);
    }
    catch (std::exception const& ex)
    {
        std::cerr << ex.what() << "\n";
        REQUIRE(false);
    }
}

SECTION("two points") {
    try
    {
        fake_path path = { 0, 0 , 1, 0};
        mapnik::extend_converter<fake_path> c(path, 1000);
        double x, y;
        REQUIRE(c.vertex(&x, &y) == mapnik::SEG_MOVETO);
        REQUIRE(x == -1000);
        REQUIRE(y == 0);
        REQUIRE(c.vertex(&x, &y) == mapnik::SEG_LINETO);
        REQUIRE(x == 1001);
        REQUIRE(y == 0);
        REQUIRE(c.vertex(&x, &y) == mapnik::SEG_END);
    }
    catch (std::exception const& ex)
    {
        std::cerr << ex.what() << "\n";
        REQUIRE(false);
    }
}

SECTION("three points") {
    try
    {
        fake_path path = { 0, 0, 1, 0, 2, 0 };
        mapnik::extend_converter<fake_path> c(path, 1000);
        double x, y;
        REQUIRE(c.vertex(&x, &y) == mapnik::SEG_MOVETO);
        REQUIRE(x == -1000);
        REQUIRE(y == 0);
        REQUIRE(c.vertex(&x, &y) == mapnik::SEG_LINETO);
        REQUIRE(x == 1);
        REQUIRE(y == 0);
        REQUIRE(c.vertex(&x, &y) == mapnik::SEG_LINETO);
        REQUIRE(x == 1002);
        REQUIRE(y == 0);
        REQUIRE(c.vertex(&x, &y) == mapnik::SEG_END);
    }
    catch (std::exception const& ex)
    {
        std::cerr << ex.what() << "\n";
        REQUIRE(false);
    }
}

SECTION("more points") {
    try
    {
        fake_path path = { 0, 0, 1, 0, 2, 0, 3, 0, 4, 0, 5, 0 };
        mapnik::extend_converter<fake_path> c(path, 1000);
        double x, y;
        REQUIRE(c.vertex(&x, &y) == mapnik::SEG_MOVETO);
        REQUIRE(x == -1000);
        REQUIRE(y == 0);
        REQUIRE(c.vertex(&x, &y) == mapnik::SEG_LINETO);
        REQUIRE(x == 1);
        REQUIRE(y == 0);
        REQUIRE(c.vertex(&x, &y) == mapnik::SEG_LINETO);
        REQUIRE(x == 2);
        REQUIRE(y == 0);
        REQUIRE(c.vertex(&x, &y) == mapnik::SEG_LINETO);
        REQUIRE(x == 3);
        REQUIRE(y == 0);
        REQUIRE(c.vertex(&x, &y) == mapnik::SEG_LINETO);
        REQUIRE(x == 4);
        REQUIRE(y == 0);
        REQUIRE(c.vertex(&x, &y) == mapnik::SEG_LINETO);
        REQUIRE(x == 1005);
        REQUIRE(y == 0);
        REQUIRE(c.vertex(&x, &y) == mapnik::SEG_END);
    }
    catch (std::exception const& ex)
    {
        std::cerr << ex.what() << "\n";
        REQUIRE(false);
    }
}

}

}
