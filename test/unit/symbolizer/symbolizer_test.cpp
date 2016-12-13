
#include "catch.hpp"

#include <iostream>
#include <mapnik/symbolizer.hpp>

using namespace mapnik;

TEST_CASE("symbolizer") {

SECTION("enums") {

    try {
        multi_policy_enum policy_in = WHOLE_MULTI;
        REQUIRE(policy_in == WHOLE_MULTI);
        markers_symbolizer sym;
        put(sym, keys::multipolicy, policy_in);
        REQUIRE(sym.properties.count(keys::multipolicy) == static_cast<unsigned long>(1));
        multi_policy_enum policy_out = get<mapnik::multi_policy_enum>(sym, keys::multipolicy);
        REQUIRE(policy_out == WHOLE_MULTI);
    }
    catch (std::exception const & ex)
    {
        std::clog << ex.what() << std::endl;
        REQUIRE(false);
    }

}
}
