#define CATCH_CONFIG_MAIN
#include <catch.hpp>
#include "CallDB.hpp"
#include "Process.hpp"


Call isend(Process &p, int pid) {
    return p.isend(0, 0, pid, 0, 0);
}

Call irecv(Process &p, WInt pid) {
    return p.irecv(0, 0, pid, WInt(0), 0);
}

Call irecv(Process &p, int pid) {
    return irecv(p, WInt(pid));
}


TEST_CASE("recev-any-1") {
    Process P0(0), P1(1), P2(2);

    vector<Call> trace;
    // P0:
    Call c1 = isend(P0, P2.pid);
    trace.push_back(c1);

    // P1:
    Call c2 = isend(P1, P2.pid);
    trace.push_back(c2);

    // P2:
    Call c3 = irecv(P2, WILDCARD);
    trace.push_back(c3);

    Schedule sched;
    sched.calls = trace;
    CallDB db(sched);

    REQUIRE(1 == db.findReceiveAny().size());
    REQUIRE(2 == db.matchReceiveAny(c3).size());
}

TEST_CASE("finalize") {
    Schedule s;
    s.procs = 3;
    Process P0(0), P1(1), P2(2);
    // P0:
    s.calls.push_back(P0.finalize());
    REQUIRE(CallDB(s).findFinalize().empty());
    REQUIRE(CallDB(s).getFinalize().size() == 1);
    s.calls.push_back(P1.finalize());
    REQUIRE(CallDB(s).getFinalize().size() == 2);
    REQUIRE(CallDB(s).findFinalize().empty());
    s.calls.push_back(P2.finalize());
    REQUIRE(CallDB(s).getFinalize().size() == 3);
    REQUIRE(CallDB(s).findFinalize().size() == 3);
}

TEST_CASE("barrier") {
    Schedule s;
    s.procs = 3;
    s.participants[10] = 3;
    Process P0(0), P1(1), P2(2);
    // P0:
    s.calls.push_back(P0.barrier(10));
    REQUIRE(CallDB(s).getCollective(CallType::BARRIER, 10).size() == 1);
    REQUIRE(CallDB(s).findCollective().empty());
    s.calls.push_back(P1.barrier(10));
    REQUIRE(CallDB(s).getCollective(CallType::BARRIER, 10).size() == 2);
    REQUIRE(CallDB(s).findCollective().empty());
    s.calls.push_back(P2.barrier(10));
    REQUIRE(CallDB(s).getCollective(CallType::BARRIER, 10).size() == 3);
    REQUIRE(CallDB(s).participantsFor(10) == 3);
    REQUIRE(CallDB(s).findCollective().size() == 3);
}

TEST_CASE("regression-1") {
    int COMM_WORLD = 0;
    Schedule s;
    s.procs = 3;
    s.participants[0] = 3; // barrier is registered with communicator 0
    Process P0(0), P1(1), P2(2);
    // P0:
    Call c1 = isend(P0, P1.pid);
    s.calls.push_back(c1);
    Call c2 = P0.barrier(COMM_WORLD);
    s.calls.push_back(c2);
    // P1:
    Call c4 = irecv(P1, WILDCARD);
    s.calls.push_back(c4);
    Call c5 = P1.barrier(COMM_WORLD);
    s.calls.push_back(c5);
    // P2:
    Call c7 = P2.barrier(COMM_WORLD);
    s.calls.push_back(c7);
    CallDB db(s);
    REQUIRE(db.findCollective().size() == 3);
}

TEST_CASE("regression-2") {
    Process P0(0), P1(1), P2(2);
    Schedule s;
    s.procs = 3;
    s.participants[0] = 3; // barrier is registered with communicator 0

    P0.curr_handle = 1;
    Call c1 = isend(P0, P1.pid);
    s.calls.push_back(c1);
    Call c3 = P0.wait(c1.handle);
    s.calls.push_back(c3);

    P1.curr_handle = 1;
    Call c4 = irecv(P1, WILDCARD);
    s.calls.push_back(c4);
    Call c6 = P1.wait(c4.handle);
    s.calls.push_back(c6);

    P0.curr_handle = 1;
    Call c8 = isend(P2, P1.pid);
    s.calls.push_back(c8);
    Call c9 = P2.wait(c8.handle);
    s.calls.push_back(c9);

    CallDB db(s);
    REQUIRE(db.matchReceiveAny(c4).size() == 2);
}
