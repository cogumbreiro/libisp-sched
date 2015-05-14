#define CATCH_CONFIG_MAIN
#include <catch.hpp>

#include "Generator.hpp"

// DOI: 10.1007/978-3-642-11261-4_12
TEST_CASE("ISP Tool Update: Scalable MPI Verification Fig.12.1 step-1") {
    /*
     * P0: Isend(to P1, &h0) ; Barrier;
     * P1: Irecv(*, &h1)     ; Barrier;
     * P2: Barrier;
     */
    Process P0(0), P1(1), P2(2);
    set<Call> trace;
    // P0:
    Call c1 = P0.isend(P1.pid);
    trace.insert(c1);
    Call c2 = P0.barrier();
    trace.insert(c2);
    REQUIRE(! c1.completesBefore(c2));
    REQUIRE(! c2.completesBefore(c1));
    // P1:
    Call c4 = P1.irecv(WILDCARD);
    trace.insert(c4);
    Call c5 = P1.barrier();
    trace.insert(c5);
    REQUIRE(! c4.completesBefore(c5));
    REQUIRE(! c5.completesBefore(c4));
    // P2:
    Call c7 = P2.barrier();
    trace.insert(c7);

    auto ms = get_match_sets(trace);
    // the program is *deterministic*, so only one trace is allowed
    REQUIRE(1 == ms.size());
    auto tmp = *ms.begin();
    auto inter = tmp.toVector();
    REQUIRE(inter.size() == 3);
    REQUIRE(inter[0] == c2); // P0:Barrier
    REQUIRE(inter[1] == c5); // P1:Barrier
    REQUIRE(inter[2] == c7); // P2:Barrier
}

// DOI: 10.1007/978-3-642-11261-4_12
TEST_CASE("ISP Tool Update: Scalable MPI Verification Fig.12.1 step-2") {
    /*
     * P0: Wait(h0);
     * P1: Wait(h1);
     * P2: Wait(h2);
     */
    Process P0(0), P1(1), P2(2);
    // GET THE SECOND PHASE ONCE ALL PROCESSES ARE BLOCKED
    set<Call> trace;
    // P0:
    P0.curr_handle = 2;
    Call c3 = P0.wait(0);
    trace.insert(c3);
    // P1:
    P1.curr_handle = 2;
    Call c6 = P1.wait(0);
    trace.insert(c6);
    // P2:
    P2.curr_handle = 2;
    Call c9 = P2.wait(1);
    trace.insert(c9);

    auto ms = get_match_sets(trace);
    // the program is deterministic
    REQUIRE(1 == ms.size());
    // one where P1 receives a message from P0
    // notice how the IReceive any gets rewritten to receive from P0
    auto inter = ms[0].toVector();
    REQUIRE(inter.size() == 3);
    REQUIRE(inter[0] == c3); // P0:Wait
    REQUIRE(inter[1] == c6); // P1:Wait
    REQUIRE(inter[2] == c9); // P2:Wait
}

// DOI: 10.1007/978-3-642-11261-4_12
TEST_CASE("ISP Tool Update: Scalable MPI Verification Fig.12.1 step-3") {
    /*
     * P0: Isend(to P1, &h0) ; Wait(h0);
     * P1: Irecv(*, &h1)     ; Wait(h1);
     * P2: Isend(to P1, &h2); Wait(h2);
     */
    Process P0(0), P1(1), P2(2);
    // GET THE SECOND PHASE ONCE ALL PROCESSES ARE BLOCKED
    set<Call> trace;
    // P0:
    P0.curr_handle = 1;
    Call c1 = P0.isend(P1.pid);
    trace.insert(c1);
    Call c3 = P0.wait(c1.handle);
    trace.insert(c3);
    // P1:
    P1.curr_handle = 1;
    Call c4 = P1.irecv(WILDCARD);
    trace.insert(c4);
    Call c6 = P1.wait(c4.handle);
    trace.insert(c6);
    // P2:
    P0.curr_handle = 1;
    Call c8 = P2.isend(P1.pid);
    trace.insert(c8);
    Call c9 = P2.wait(c8.handle);
    trace.insert(c9);

    auto ms = get_match_sets(trace);
    // the program is nondeterministic; two traces are allowed
    REQUIRE(2 == ms.size());
    {
        // one where P1 receives a message from P0
        // notice how the IReceive any gets rewritten to receive from P0
        auto inter = ms[0].toVector();
        REQUIRE(inter.size() == 2);
        auto c4_aux = c4;
        c4_aux.recv.src = P0.pid;
        REQUIRE(inter[0] == c4_aux); // P1:IReceive(0)
        REQUIRE(inter[1] == c1); // P0:ISend(P1)
    }
    {
        // and another one where P1 receives from P2
        // notice how the IReceive any gets rewritten to receive from P2
        auto inter = ms[1].toVector();
        REQUIRE(inter.size() == 2);
        auto c4_aux = c4;
        c4_aux.recv.src = P2.pid;
        REQUIRE(inter[0] == c4_aux); // P1:IReceive(P2)
        REQUIRE(inter[1] == c8); // P2:ISend(P1)
    }
}
