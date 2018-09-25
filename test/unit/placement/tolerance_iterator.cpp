#include "catch.hpp"

#include <mapnik/tolerance_iterator.hpp>

#include <array>

TEST_CASE("tolerance_iterator") {

SECTION("exponential_function") {

    const mapnik::exponential_function func;
    const double tolerance = 100.0;
    const std::array<double, 26> expected_values =
    {
        1.00325, 2.00845, 3.01648, 4.02856,
        5.04641, 6.0724, 7.10981, 8.16315,
        9.2386, 10.3446, 11.4928, 12.6989,
        13.9843, 15.3781, 16.9195, 18.6617,
        20.6764, 23.0605, 25.9441, 29.5025,
        33.9709, 39.6651, 47.0085, 56.568,
        69.1026, 85.6267
    };

    {
        int index = 0;
        for (double p = 1.0, value = func(p, tolerance);
            p < tolerance && value < tolerance;
            ++index, p += 1.0, value = func(p, tolerance))
        {
            REQUIRE(index < expected_values.size());
            CHECK(value == Approx(expected_values[index]));
        }
    }

    {
        using iterator_type = mapnik::tolerance_iterator<
            mapnik::exponential_function>;

        iterator_type iterator(tolerance, func);
        REQUIRE(iterator.next());
        REQUIRE(iterator.get() == Approx(0.0));

        for (int i = 0; iterator.next(); ++i)
        {
            int index = i / 2;
            REQUIRE(index < expected_values.size());
            double expected_value = expected_values[index] *
                ((i % 2) ? 1 : -1);
            CHECK(iterator.get() == Approx(expected_value));
        }
    }
}

}
