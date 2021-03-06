#define CATCH_CONFIG_MAIN
#include <catch.hpp>

#include "Generator.hpp"
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

// DOI: 10.1007/978-3-642-11261-4_12
TEST_CASE("ISP Tool Update: Scalable MPI Verification Fig.12.1 step-1") {
    /*
     * P0: Isend(to P1, &h0) ; Barrier;
     * P1: Irecv(*, &h1)     ; Barrier;
     * P2: Barrier;
     */
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

    auto ms = get_match_sets(s);
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
    Schedule s;
    s.procs = 3;
    s.participants[0] = 3; // barrier is registered with communicator 0

    // GET THE SECOND PHASE ONCE ALL PROCESSES ARE BLOCKED
    // P0:
    P0.curr_handle = 2;
    Call c3 = P0.wait(0);
    s.calls.push_back(c3);
    // P1:
    P1.curr_handle = 2;
    Call c6 = P1.wait(0);
    s.calls.push_back(c6);
    // P2:
    P2.curr_handle = 2;
    Call c9 = P2.wait(1);
    s.calls.push_back(c9);

    auto ms = get_match_sets(s);
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
     * P2: Isend(to P1, &h2) ; Wait(h2);
     */
    Process P0(0), P1(1), P2(2);
    Schedule s;
    s.procs = 3;
    s.participants[0] = 3; // barrier is registered with communicator 0
    // GET THE THIRD PHASE ONCE ALL PROCESSES ARE BLOCKED

    // P0:
    P0.curr_handle = 1;
    Call c1 = isend(P0, P1.pid);
    s.calls.push_back(c1);
    Call c3 = P0.wait(c1.handle);
    s.calls.push_back(c3);
    // P1:
    P1.curr_handle = 1;
    Call c4 = irecv(P1, WILDCARD);
    s.calls.push_back(c4);
    Call c6 = P1.wait(c4.handle);
    s.calls.push_back(c6);
    // P2:
    P0.curr_handle = 1;
    Call c8 = isend(P2, P1.pid);
    s.calls.push_back(c8);
    Call c9 = P2.wait(c8.handle);
    s.calls.push_back(c9);

    auto ms = get_match_sets(s);
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
