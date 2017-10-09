#include "catch.hpp"

#include <mapnik/vertex_adapters.hpp>
#include <mapnik/geometry/interior.hpp>

TEST_CASE("interior") {

/*
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
    REQUIRE(mapnik::geometry::interior(va, interior.x, interior.y));
    REQUIRE(interior.x == 0.5);
    REQUIRE(interior.y == 0.5);
}
*/

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
    REQUIRE(mapnik::geometry::interior(va, interior.x, interior.y));
    REQUIRE(interior.x == 0.0);
    REQUIRE(interior.y == 0.0);
}

SECTION("bisector crosses vertex") {

    mapnik::geometry::polygon<double> poly;
    mapnik::geometry::linear_ring<double> ring;
    ring.emplace_back(1, 0);
    ring.emplace_back(0, 1);
    ring.emplace_back(-1, 0);
    ring.emplace_back(0, -1);
    ring.emplace_back(1, 0);
    poly.push_back(std::move(ring));

    mapnik::geometry::polygon_vertex_adapter<double> va(poly);

    using bisector_type = mapnik::geometry::detail::bisector;
    using intersection_type = mapnik::geometry::detail::intersection;
    using point_type = bisector_type::point_type;

    const point_type center(0, 0);
    const double angle = 0;

    std::vector<bisector_type> bisectors;
    bisectors.emplace_back(center, angle);

    std::vector<std::vector<intersection_type>> intersections_per_bisector(bisectors.size());
    mapnik::geometry::detail::make_intersections(va, bisectors, intersections_per_bisector, center);

    std::vector<intersection_type> const& intersections = intersections_per_bisector.front();

    REQUIRE(intersections.size() == 2);
}

SECTION("segment parallel to bisector") {

    mapnik::geometry::polygon<double> poly;
    mapnik::geometry::linear_ring<double> ring;
    ring.emplace_back(0, 0);
    ring.emplace_back(1, 0);
    ring.emplace_back(1, 1);
    ring.emplace_back(0, 1);
    ring.emplace_back(0, 0);
    poly.push_back(std::move(ring));

    mapnik::geometry::polygon_vertex_adapter<double> va(poly);

    using bisector_type = mapnik::geometry::detail::bisector;
    using intersection_type = mapnik::geometry::detail::intersection;
    using point_type = bisector_type::point_type;

    const point_type center(0.5, 1);
    const double angle = 0;

    std::vector<bisector_type> bisectors;
    bisectors.emplace_back(center, angle);

    std::vector<std::vector<intersection_type>> intersections_per_bisector(bisectors.size());
    mapnik::geometry::detail::make_intersections(va, bisectors, intersections_per_bisector, center);

    std::vector<intersection_type> const& intersections = intersections_per_bisector.front();

    REQUIRE(intersections.size() == 2);
}

/*
SECTION("bisector") {

    using bisector_type = mapnik::geometry::detail::bisector;
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
*/

}
