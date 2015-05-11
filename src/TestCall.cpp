#define CATCH_CONFIG_MAIN
#include <catch.hpp>

#include "Call.hpp"

TEST_CASE("Testing Call::equals") {
    REQUIRE(Call(1, 10, Envelope()) == Call(1, 10, Envelope()));
    REQUIRE(Call(1, 5, Envelope()) == Call(1, 5, Envelope()));
    REQUIRE(Call(0, 2, Envelope()) == Call(0, 2, Envelope()));
}

TEST_CASE("Testing Call::neq") {
    REQUIRE(Call(1, 10, Envelope()) != Call(1, 0, Envelope()));
    REQUIRE(Call(1, 5, Envelope()) != Call(2, 5, Envelope()));
}

TEST_CASE("Testing Call::<") {
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

TEST_CASE("Regression 1") {
    const int P0 = 0, P1 = 1, P2 = 2;
    Call c1(P0, 0, Envelope::ISend(P1));
    Call c3(P0, 2, Envelope::Wait(0));
    REQUIRE(c1.envelope.func_id == OpType::ISEND);
    REQUIRE(c3.envelope.func_id == OpType::WAIT);
    REQUIRE(c1.envelope.count == 0);
    REQUIRE(c3.envelope.count == 0);
    REQUIRE(c1.envelope.completesBefore(c3.envelope));
    REQUIRE(c1.envelope == Envelope::ISend(P1));
    REQUIRE(c1.completesBefore(c3));
}
