#include "catch.hpp"

#include <vector>
#include <mapnik/util/alternating_iterator.hpp>

TEST_CASE("alternating_iterator") {

SECTION("empty container") {
    std::vector<int> v;
    mapnik::alternating_iterator<int, std::vector> i(v, 0);
    mapnik::alternating_iterator<int, std::vector> e(v);

    REQUIRE(!(i != e));
}

SECTION("one element") {
    std::vector<int> v { 9 };
    mapnik::alternating_iterator<int, std::vector> i(v, 0);
    mapnik::alternating_iterator<int, std::vector> e(v);

    REQUIRE(*i == 9);
    ++i;
    REQUIRE(!(i != e));
}

SECTION("iterations 1") {
    std::vector<int> v { 9, 8, 7, 6 };
    mapnik::alternating_iterator<int, std::vector> i(v, 0);
    mapnik::alternating_iterator<int, std::vector> e(v);

    REQUIRE(*i == 9);
    ++i;
    REQUIRE(*i == 8);
    ++i;
    REQUIRE(*i == 7);
    ++i;
    REQUIRE(*i == 6);
    ++i;
    REQUIRE(!(i != e));
}

SECTION("iterations 2") {
    std::vector<int> v { 9, 8, 7, 6 };
    mapnik::alternating_iterator<int, std::vector> i(v, 3);
    mapnik::alternating_iterator<int, std::vector> e(v);

    REQUIRE(*i == 6);
    ++i;
    REQUIRE(*i == 7);
    ++i;
    REQUIRE(*i == 8);
    ++i;
    REQUIRE(*i == 9);
    ++i;
    REQUIRE(!(i != e));
}

SECTION("iterations 3") {
    std::vector<int> v { 9, 8, 7, 6, 5, 4, 3, 2, 1, 0 };
    mapnik::alternating_iterator<int, std::vector> i(v, 2);
    mapnik::alternating_iterator<int, std::vector> e(v);

    REQUIRE(*i == 7);
    ++i;
    REQUIRE(*i == 6);
    ++i;
    REQUIRE(*i == 8);
    ++i;
    REQUIRE(*i == 5);
    ++i;
    REQUIRE(*i == 9);
    ++i;
    REQUIRE(*i == 4);
    ++i;
    REQUIRE(*i == 3);
    ++i;
    REQUIRE(*i == 2);
    ++i;
    REQUIRE(*i == 1);
    ++i;
    REQUIRE(*i == 0);
    ++i;
    REQUIRE(!(i != e));
}

SECTION("iterations 4") {
    std::vector<int> v { 9, 8, 7, 6, 5, 4, 3, 2, 1, 0 };
    mapnik::alternating_iterator<int, std::vector> i(v, 8);
    mapnik::alternating_iterator<int, std::vector> e(v);

    REQUIRE(*i == 1);
    ++i;
    REQUIRE(*i == 0);
    ++i;
    REQUIRE(*i == 2);
    ++i;
    REQUIRE(*i == 3);
    ++i;
    REQUIRE(*i == 4);
    ++i;
    REQUIRE(*i == 5);
    ++i;
    REQUIRE(*i == 6);
    ++i;
    REQUIRE(*i == 7);
    ++i;
    REQUIRE(*i == 8);
    ++i;
    REQUIRE(*i == 9);
    ++i;
    REQUIRE(!(i != e));
}

} // TEST_CASE
