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

TEST_CASE("regression-1") {
    const int P0 = 0, P1 = 1, P2 = 2;
    Call c1(P0, 0, Envelope::ISend(P1));
    Call c3(P0, 2, Envelope::Wait(0));
    REQUIRE(c1.envelope.call_type == OpType::ISEND);
    REQUIRE(c3.envelope.call_type == OpType::WAIT);
    REQUIRE(c1.envelope.count == 0);
    REQUIRE(c3.envelope.count == 0);
    REQUIRE(c1.envelope.completesBefore(c3.envelope));
    REQUIRE(c1.envelope == Envelope::ISend(P1));
    REQUIRE(c1.completesBefore(c3));
}

TEST_CASE("regression-2") {
    const int P0 = 0, P1 = 1, P2 = 2;
    // GET THE SECOND PHASE ONCE ALL PROCESSES ARE BLOCKED
    set<Call> trace;
    // P0:
    Call c1(P0, 0, Envelope::ISend(P1));
    trace.insert(c1);
    Call c3(P0, 2, Envelope::Wait(0));
    REQUIRE(c1.pid == c3.pid);
    REQUIRE(c3.handle >= c1.handle);
    REQUIRE(c1.completesBefore(c3));
    REQUIRE(c3.hasAncestors(trace));
}
