#define CATCH_CONFIG_MAIN
#include <catch.hpp>

#include "Generator.hpp"


TEST_CASE("regression-1") {
    Process P0(0), P1(1), P2(2);
    set<Call> trace;
    Call c1 = P0.ISend(P1.pid);
    trace.insert(c1);
    Call c4 = P1.IRecv(WILDCARD);
    trace.insert(c4);
    Call c8 = P2.ISend(P1.pid);
    trace.insert(c8);

    auto ms = get_match_sets(trace);
    REQUIRE(2 == ms.size());
    {
        auto inter = ms[0].toVector();
        REQUIRE(inter.size() == 2);
        auto c4_aux = c4;
        c4_aux.recv.src = P0.pid;
        REQUIRE(inter[0] == c4_aux); // P1:IReceive(P0)
        REQUIRE(inter[1] == c1); // P0:ISend(P1)
    }
    {
        auto inter = ms[1].toVector();
        REQUIRE(inter.size() == 2);
        // notice how the IReceive any gets rewritten to receive from P2
        auto c4_aux = c4;
        c4_aux.recv.src = P2.pid;
        REQUIRE(inter[0] == c4_aux); // P1:IReceive
        REQUIRE(inter[1] == c8); // P2:ISend(P1)
    }
}


TEST_CASE("recev-any-1") {
    Process P0(0), P1(1), P2(2);

    set<Call> trace;
    // P0:
    Call c1 = P0.ISend(P2.pid);
    trace.insert(c1);

    // P1:
    Call c2 = P1.ISend(P2.pid);
    trace.insert(c2);

    // P2:
    Call c3 = P2.IRecv(WILDCARD);
    trace.insert(c3);

    auto ms = get_match_sets(trace);
    // the receive any forks 2 states:
    REQUIRE(2 == ms.size());
    auto inter = ms[0].toVector();
    REQUIRE(inter.size() == 2);
    auto c3_1 = c3;
    c3_1.recv.src = P0.pid;
    REQUIRE(inter[0] == c3_1);
    REQUIRE(inter[1] == c1);
    inter = ms[1].toVector();
    REQUIRE(inter.size() == 2);
    auto c3_2 = c3;
    c3_2.recv.src = P1.pid;
    REQUIRE(inter[0] == c3_2);
    REQUIRE(inter[1] == c2);
}

TEST_CASE("permutations-1") {
    Process P0(0), P1(1), P2(2);

    // P0:
    Call c1 = P0.ISend(P0.pid);
    MatchSet s1;
    s1.add(c1);

    // P1:
    Call c2 = P1.ISend(P1.pid);
    MatchSet s2;
    s2.add(c2);
    vector<MatchSet> v1 {s1, s2};

    // P2:
    Call c3 = P2.IRecv(WILDCARD);
    MatchSet s3;
    s3.add(c3);

    // P2:
    Call c4 = P2.IRecv(WILDCARD);
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
