#define CATCH_CONFIG_MAIN 
#include <catch.hpp>
#include "CB.hpp"

TEST_CASE("Testing CB", "[wat]") {
    // only smaller if the first component (the pid) is smaller
    REQUIRE(CB(0, 100) < CB(1, 0));
    // if pid is the same, then compare 
    REQUIRE(CB(1, 0) < CB(1, 100));
    // equality is defined
    REQUIRE(CB(100, 100) == CB(100, 100));
}


