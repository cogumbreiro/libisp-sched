#define CATCH_CONFIG_MAIN
#include <catch.hpp>

#include "Call.hpp"

TEST_CASE("Testing Call::equals") {
    REQUIRE(Call(1, 10) == Call(1, 10));
    REQUIRE(Call(1, 5) == Call(1, 5));
    REQUIRE(Call(0, 2) == Call(0, 2));
}

TEST_CASE("Testing Call::neq") {
    REQUIRE(Call(1, 10) != Call(1, 0));
    REQUIRE(Call(1, 5) != Call(2, 5));
}

TEST_CASE("Testing Call::<") {
    REQUIRE(!(Call(1, 10) < Call(1, 10)));
    REQUIRE(!(Call(1, 10) < Call(1, 5)));
    REQUIRE(!(Call(1, 10) < Call(0, 2)));


    REQUIRE(!(Call(1, 50) < Call(1, 10)));
    REQUIRE(!(Call(1, 5) < Call(1, 5)));
    REQUIRE(!(Call(1, 5) < Call(0, 2)));

    REQUIRE((Call(0, 2) < Call(1, 10)));
    REQUIRE((Call(0, 2) < Call(1, 5)));
    REQUIRE(!(Call(0, 2) < Call(0, 2)));
}

TEST_CASE("regression-1") {
    auto P0 = Process(0), P1 = Process(1), P2 = Process(2);
    Call c1 = P0.ISend(P1.pid);
    Call c3 = P0.Wait(c1.handle);
    REQUIRE(c1.call_type == OpType::ISEND);
    REQUIRE(c3.call_type == OpType::WAIT);
    REQUIRE(c1.send.count == 0);
    REQUIRE(c1 < c3);
    REQUIRE(c1.completesBefore(c3));
//    REQUIRE(c1.envelope == Envelope::ISend(P1));
//    REQUIRE(c1.completesBefore(c3));
}

TEST_CASE("regression-2") {
    auto P0 = Process(0), P1 = Process(1), P2 = Process(2);
    // GET THE SECOND PHASE ONCE ALL PROCESSES ARE BLOCKED
    set<Call> trace;
    // P0:
    Call c1 = P0.ISend(P1.pid);
    trace.insert(c1);
    Call c3 = P0.Wait(c1.handle);
    REQUIRE(c1.pid == c3.pid);
    REQUIRE(c1.handle < c3.handle);
    REQUIRE(c1.completesBefore(c3));
    REQUIRE(c3.hasAncestors(trace));
}
