#define CATCH_CONFIG_MAIN
#include <catch.hpp>

#include "Generator.hpp"

// DOI: 10.1007/978-3-642-11261-4_12
TEST_CASE("ISP Tool Update: Scalable MPI Verification example-1") {
    /*
     * P0: Isend(to P1, &h0) ; Barrier; Wait(h0);
     * P1: Irecv(*, &h1)     ; Barrier; Wait(h1);
     * P2: Barrier; Isend(to P1, &h2); Wait(h2);
     */
    const int P0 = 0, P1 = 1, P2 = 2;
    // GET THE SCHEDULE WHERE ALL PROCESSES ARE BLOCKED
    set<Call> trace;
    // P0:
    Call c1(P0, 0, Envelope::ISend(P1));
    trace.insert(c1);
    Call c2(P0, 1, Envelope::Barrier());
    trace.insert(c2);
    //Call c3(P0, 2, Envelope::Wait(0));
    //trace.insert(c3);
    REQUIRE(! c1.completesBefore(c2));
    REQUIRE(! c2.completesBefore(c1));

    // P1:
    Call c4(P1, 0, Envelope::IRecv(WILDCARD));
    trace.insert(c4);
    Call c5(P1, 1, Envelope::Barrier());
    trace.insert(c5);
    //Call c6(P1, 2, Envelope::Wait(0));
    //trace.insert(c6);
    REQUIRE(! c4.completesBefore(c5));
    REQUIRE(! c5.completesBefore(c4));
    // P2:
    Call c7(P2, 0, Envelope::Barrier());
    trace.insert(c7);
    //Call c8(P2, 1, Envelope::ISend(P1));
    //trace.insert(c8);
    //Call c9(P2, 2, Envelope::Wait(1));
    //trace.insert(c9);
    /*
    // All calls must be enabled
    auto procs = sort_by_procs(trace);

    set<Call> proc0 = filter_enabled(procs[P0]);
    REQUIRE(2 == proc0.size());
    REQUIRE(proc0.find(c1) != proc0.end());
    REQUIRE(proc0.find(c2) != proc0.end());

    set<Call> proc1 = filter_enabled(procs[P1]);
    REQUIRE(2 == proc1.size());
    REQUIRE(proc1.find(c4) != proc1.end());
    REQUIRE(proc1.find(c5) != proc1.end());

    set<Call> proc2 = filter_enabled(procs[P2]);
    REQUIRE(1 == proc2.size());
    REQUIRE(proc2.find(c7) != proc2.end());
    */

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

TEST_CASE("recev-any-1") {
    const int P0 = 0, P1 = 1, P2 = 2;

    set<Call> trace;
    // P0:
    Call c1(P0, 0, Envelope::ISend(P2));
    trace.insert(c1);

    // P1:
    Call c2(P1, 0, Envelope::ISend(P2));
    trace.insert(c2);

    // P2:
    Call c3(P2, 0, Envelope::IRecv(WILDCARD));
    trace.insert(c3);

    Generator g(trace);
    REQUIRE(g.at(MPIKind::ReceiveAny).size() == 1);
    REQUIRE(g.at(MPIKind::Send).size() == 2);
    REQUIRE(c1.envelope.canSend(c3.envelope));
    REQUIRE(c2.envelope.canSend(c3.envelope));
    REQUIRE(get_sends_for(c3.envelope, g.at(MPIKind::Send)).size() == 2);

    auto ms = get_match_sets(trace);
    // the receive any forks 2 states:
    REQUIRE(2 == ms.size());
    auto inter = ms[0].toVector();
    REQUIRE(inter.size() == 2);
    REQUIRE(inter[0] == Call(P2, 0, Envelope::IRecv(P0)));
    REQUIRE(inter[1] == c1);
    inter = ms[1].toVector();
    REQUIRE(inter.size() == 2);
    REQUIRE(inter[0] == Call(P2, 0, Envelope::IRecv(P1)));
    REQUIRE(inter[1] == c2);
}