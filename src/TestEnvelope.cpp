#define CATCH_CONFIG_MAIN
#include <catch.hpp>

#include "Call.hpp"

TEST_CASE("Regression 2") {
    REQUIRE(Envelope::ISend(0) == Envelope::ISend(0));
}

TEST_CASE("Regression 1") {
    const int P0 = 0, P1 = 1, P2 = 2;
    Envelope e1 = Envelope::ISend(P1);
    Envelope e3 = Envelope::Wait(0);
    REQUIRE(e1.completesBefore(e3));
}
