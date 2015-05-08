#define CATCH_CONFIG_MAIN
#include <catch.hpp>
#include "Call.hpp"

TEST_CASE("Testing Call::equals", "[call]") {
    REQUIRE(Call(1, 10, Envelope()) == Call(1, 10, Envelope()));
    REQUIRE(Call(1, 5, Envelope()) == Call(1, 5, Envelope()));
    REQUIRE(Call(0, 2, Envelope()) == Call(0, 2, Envelope()));
    // neq
    REQUIRE(Call(1, 10, Envelope()) != Call(1, 0, Envelope()));
    REQUIRE(Call(1, 5, Envelope()) != Call(2, 5, Envelope()));
}

TEST_CASE("Testing Call::<", "[call]") {
    REQUIRE(!(Call(1, 10, Envelope()) < Call(1, 10, Envelope())));
    REQUIRE(!(Call(1, 10, Envelope()) < Call(1, 5, Envelope())));
    REQUIRE(!(Call(1, 10, Envelope()) < Call(0, 2, Envelope())));


    REQUIRE((Call(1, 5, Envelope()) < Call(1, 10, Envelope())));
    REQUIRE(!(Call(1, 5, Envelope()) < Call(1, 5, Envelope())));
    REQUIRE(!(Call(1, 5, Envelope()) < Call(0, 2, Envelope())));

    REQUIRE((Call(0, 2, Envelope()) < Call(1, 10, Envelope())));
    REQUIRE((Call(0, 2, Envelope()) < Call(1, 5, Envelope())));
    REQUIRE(!(Call(0, 2, Envelope()) < Call(0, 2, Envelope())));
}
