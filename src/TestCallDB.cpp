#define CATCH_CONFIG_MAIN
#include <catch.hpp>
#include "CallDB.hpp"

TEST_CASE("recev-any-1") {
    Process P0(0), P1(1), P2(2);

    set<Call> trace;
    // P0:
    Call c1 = P0.isend(P2.pid);
    trace.insert(c1);

    // P1:
    Call c2 = P1.isend(P2.pid);
    trace.insert(c2);

    // P2:
    Call c3 = P2.irecv(WILDCARD);
    trace.insert(c3);

    CallDB db(trace);

    REQUIRE(1 == db.findReceiveAny().size());
    REQUIRE(2 == db.matchReceiveAny(c3).size());
}
