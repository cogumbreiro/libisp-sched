#define CATCH_CONFIG_MAIN
#include <catch.hpp>
#include "CallDB.hpp"

TEST_CASE("recev-any-1") {
    Process P0(0), P1(1), P2(2);

    vector<Call> trace;
    // P0:
    Call c1 = P0.isend(P2.pid);
    trace.push_back(c1);

    // P1:
    Call c2 = P1.isend(P2.pid);
    trace.push_back(c2);

    // P2:
    Call c3 = P2.irecv(WILDCARD);
    trace.push_back(c3);

    Schedule sched;
    sched.calls = trace;
    CallDB db(sched);

    REQUIRE(1 == db.findReceiveAny().size());
    REQUIRE(2 == db.matchReceiveAny(c3).size());
}

TEST_CASE("regression-1") {
    Schedule s;
    s.procs = 3;
    s.participants[0] = 3; // barrier is registered with communicator 0
    Process P0(0), P1(1), P2(2);
    // P0:
    Call c1 = P0.isend(P1.pid);
    s.calls.push_back(c1);
    Call c2 = P0.barrier();
    s.calls.push_back(c2);
    // P1:
    Call c4 = P1.irecv(WILDCARD);
    s.calls.push_back(c4);
    Call c5 = P1.barrier();
    s.calls.push_back(c5);
    // P2:
    Call c7 = P2.barrier();
    s.calls.push_back(c7);

}
