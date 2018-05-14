#include "catch.hpp"

#include <mapnik/util/parallelize.hpp>

TEST_CASE("parallelize") {

SECTION("jobs_by_image_size") {

    using namespace mapnik::util;
    const unsigned max_concurrency = 18;
    CHECK(jobs_by_image_size(1, 1, max_concurrency) == 1);
    CHECK(jobs_by_image_size(512, 512, max_concurrency) == 1);
    CHECK(jobs_by_image_size(1024, 1024, max_concurrency) == 1);
    CHECK(jobs_by_image_size(1024, 2048, max_concurrency) == 2);
    CHECK(jobs_by_image_size(2048, 2048, max_concurrency) == 4);
    CHECK(jobs_by_image_size(4096, 4096, max_concurrency) == 16);
    CHECK(jobs_by_image_size(4096, 4100, max_concurrency) == 16);
    CHECK(jobs_by_image_size(10000, 10000, max_concurrency) == 18);
    CHECK(jobs_by_image_size(4096, 4096, 10) == 10);
}

}
