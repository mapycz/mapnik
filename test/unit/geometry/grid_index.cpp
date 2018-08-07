#include "catch.hpp"

#include <mapnik/grid_index.hpp>

TEST_CASE("grid_index") {

SECTION("grid_index, query bbox") {
    mapnik::grid_index<int16_t> grid(100, 100, 10);
    grid.insert(0, mapnik::box2d<double>(4, 10, 6, 30));
    grid.insert(1, mapnik::box2d<double>(4, 10, 30, 12));
    grid.insert(2, mapnik::box2d<double>(-10, 30, 5, 35));
    
    CHECK(grid.query(mapnik::box2d<double>(4, 10, 5, 11)) == (std::vector<int16_t>{0, 1}));
    CHECK(grid.query(mapnik::box2d<double>(24, 10, 25, 11)) == (std::vector<int16_t>{1}));
    CHECK(grid.query(mapnik::box2d<double>(40, 40, 100, 100)) == (std::vector<int16_t>{}));
    CHECK(grid.query(mapnik::box2d<double>(-6, 0, 3, 100)) == (std::vector<int16_t>{2}));
    CHECK(grid.query(mapnik::box2d<double>(-1000, -1000, 1000, 1000)) == (std::vector<int16_t>{0, 1, 2}));
}

SECTION("grid_index, DuplicateKeys") {
    mapnik::grid_index<int16_t> grid(100, 100, 10);
    #define KEY 123
    grid.insert(KEY, mapnik::box2d<double>(3, 4, 4, 4));
    grid.insert(KEY, mapnik::box2d<double>(13, 13, 14, 14));
    grid.insert(KEY, mapnik::box2d<double>(23, 23, 24, 24));
    
    CHECK(grid.query(mapnik::box2d<double>(0, 0, 30, 30)) == (std::vector<int16_t>{KEY, KEY, KEY}));
}

}
