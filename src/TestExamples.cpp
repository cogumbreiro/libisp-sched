#define CATCH_CONFIG_MAIN
#include <catch.hpp>

#include "Generator.hpp"

// DOI: 10.1007/978-3-642-11261-4_12
TEST_CASE("ISP Tool Update: Scalable MPI Verification example-1.1") {
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
TEST_CASE("ISP Tool Update: Scalable MPI Verification example-1.2") {
    /*
     * P0: Isend(to P1, &h0) ; Wait(h0);
     * P1: Irecv(*, &h1)     ; Wait(h1);
     * P2: Isend(to P1, &h2); Wait(h2);
     */
    const int P0 = 0, P1 = 1, P2 = 2;
    // GET THE SECOND PHASE ONCE ALL PROCESSES ARE BLOCKED
    set<Call> trace;
    // P0:
    Call c1(P0, 0, Envelope::ISend(P1));
    trace.insert(c1);
    //Call c2(P0, 1, Envelope::Barrier());
    //trace.insert(c2);
    Call c3(P0, 2, Envelope::Wait(0));
    trace.insert(c3);

    // P1:
    Call c4(P1, 0, Envelope::IRecv(WILDCARD));
    trace.insert(c4);
    //Call c5(P1, 1, Envelope::Barrier());
    //trace.insert(c5);
    Call c6(P1, 2, Envelope::Wait(0));
    trace.insert(c6);
    REQUIRE(c4.completesBefore(c6));
    REQUIRE(!c6.completesBefore(c4));

    // P2:
    //Call c7(P2, 0, Envelope::Barrier());
    //trace.insert(c7);
    Call c8(P2, 1, Envelope::ISend(P1));
    trace.insert(c8);
    Call c9(P2, 2, Envelope::Wait(1));
    trace.insert(c9);
    REQUIRE(c8.completesBefore(c9));
    REQUIRE(!c9.completesBefore(c8));

    auto ms = get_match_sets(trace);
    // the program is *deterministic*, so only one trace is allowed
    REQUIRE(2 == ms.size());
    auto inter = ms[0].toVector();
    REQUIRE(inter.size() == 2);
    // notice how the IReceive any gets rewritten to receive from P0
    REQUIRE(inter[0] == Call(P1, 0, Envelope::IRecv(P0))); // P1:IReceive
    REQUIRE(inter[1] == c1); // P0:ISend(P1)

    // Next interleaving
    inter = ms[1].toVector();
    REQUIRE(inter.size() == 2);
    // notice how the IReceive any gets rewritten to receive from P2
    REQUIRE(inter[0] == Call(P2, 0, Envelope::IRecv(P2))); // P1:IReceive
    REQUIRE(inter[1] == c8); // P2:ISend(P1)
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

TEST_CASE("permutations-1") {
    const int P0 = 0, P1 = 1, P2 = 2;

    // P0:
    Call c1(P0, 0, Envelope::ISend(P0));
    MatchSet s1;
    s1.add(c1);

    // P1:
    Call c2(P1, 1, Envelope::ISend(P1));
    MatchSet s2;
    s2.add(c2);
    vector<MatchSet> v1 {s1, s2};

    // P2:
    Call c3(P2, 2, Envelope::IRecv(WILDCARD));
    MatchSet s3;
    s3.add(c3);

    // P2:
    Call c4(P2, 3, Envelope::IRecv(P0));
    MatchSet s4;
    s4.add(c4);
    vector<MatchSet> v2 {s3, s4};

    auto sets = mix(v1, v2);

    REQUIRE(sets.size() == 4);
    {
        auto p = sets[0].toVector();
        REQUIRE(p.size() == 2);
        REQUIRE(p[0] == c1);
        REQUIRE(p[1] == c3);
    }
    {
        auto p = sets[1].toVector();
        REQUIRE(p.size() == 2);
        REQUIRE(p[0] == c1);
        REQUIRE(p[1] == c4);
    }
    {
        auto p = sets[2].toVector();
        REQUIRE(p.size() == 2);
        REQUIRE(p[0] == c2);
        REQUIRE(p[1] == c3);
    }
    {
        auto p = sets[3].toVector();
        REQUIRE(p.size() == 2);
        REQUIRE(p[0] == c2);
        REQUIRE(p[1] == c4);
    }
}
