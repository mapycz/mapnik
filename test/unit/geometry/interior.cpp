#include "catch.hpp"

#include <mapnik/vertex_adapters.hpp>
#include <mapnik/geom_util.hpp>

TEST_CASE("polygon interior") {

SECTION("polygon 1") {

    mapnik::geometry::polygon<double> poly;
    mapnik::geometry::linear_ring<double> ring;
    ring.emplace_back(0, 0);
    ring.emplace_back(1, 0);
    ring.emplace_back(1, 1);
    ring.emplace_back(0, 1);
    ring.emplace_back(0, 0);
    poly.push_back(std::move(ring));

    mapnik::geometry::polygon_vertex_adapter<double> va(poly);

    mapnik::geometry::point<double> interior;
    REQUIRE(mapnik::label::interior_position(va, interior.x, interior.y));
    REQUIRE(interior.x == 0.5);
    REQUIRE(interior.y == 0.5);
}

SECTION("polygon 2") {

    mapnik::geometry::polygon<double> poly;
    mapnik::geometry::linear_ring<double> ring;
    ring.emplace_back(1, 0);
    ring.emplace_back(0, 1);
    ring.emplace_back(-1, 0);
    ring.emplace_back(0, -1);
    ring.emplace_back(1, 0);
    poly.push_back(std::move(ring));

    mapnik::geometry::polygon_vertex_adapter<double> va(poly);

    mapnik::geometry::point<double> interior;
    REQUIRE(mapnik::label::interior_position(va, interior.x, interior.y));
    REQUIRE(interior.x == 0.0);
    REQUIRE(interior.y == 0.0);
}

SECTION("bisector") {

    using bisector_type = mapnik::label::bisector;
    using point_type = bisector_type::point_type;

    {
        bisector_type bisector(point_type(0.5, 0.5), 0);
        CHECK(!bisector.intersects(point_type(0, 0), point_type(1, 0)));
        CHECK(bisector.intersects(point_type(0, 0), point_type(1, 1)));
    }

    {
        bisector_type bisector(point_type(270.20886059972594, 253.57572393803318), 3.0 * M_PI / 4.0);
        const point_type p1(179.15547826136165, 399.62713043390545);
        const point_type p2(151.90817391149596, 364.54399999915978);

        CHECK(bisector.intersects(p1, p2));
        CHECK(bisector.intersects(p2, p1));

        const point_type intersection = bisector.intersection(p1, p2);
        CHECK(intersection.x == Approx(155.1134848429));
        CHECK(intersection.y == Approx(368.6710996948));
    }

    {
        bisector_type bisector(point_type(0.5, 0.5), M_PI / 4.0);
        const point_type point(1, 1);
        const point_type rotated = bisector.rotate_back(point);
        CHECK(rotated.x == Approx(std::sqrt(2)));
        CHECK(rotated.y == Approx(0.0));
    }

    {
        bisector_type bisector(point_type(0.5, 0.5), 3.0 * M_PI / 4.0);
        const point_type point(-1, 1);
        const point_type rotated = bisector.rotate_back(point);
        CHECK(rotated.x == Approx(std::sqrt(2)));
        CHECK(rotated.y == Approx(0.0));
    }
}

}
