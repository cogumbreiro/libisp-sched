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

TEST_CASE("regression-1") {
    Process P0(0), P1(1), P2(2);
    vector<Call> trace;
    Call c1 = isend(P0, P1.pid);
    trace.push_back(c1);
    Call c4 = irecv(P1, WILDCARD);
    trace.push_back(c4);
    Call c8 = isend(P2, P1.pid);
    trace.push_back(c8);

    Schedule s;
    s.calls = trace;
    auto ms = get_match_sets(s);
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
    Generator g(db);

    REQUIRE(g.matchReceiveAny().size() == 2);
    REQUIRE(g.matchCollective().empty());
    REQUIRE(g.matchFinalize().empty());
    REQUIRE(g.matchReceive().empty());
    REQUIRE(g.matchWait().empty());
    REQUIRE(g.getMatchSets().size() == 2);

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

    Schedule s;
    s.calls = trace;
    auto ms = get_match_sets(s);
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
    Call c1 = isend(P0, P0.pid);
    MatchSet s1;
    s1.add(c1);

    // P1:
    Call c2 = isend(P1, P1.pid);
    MatchSet s2;
    s2.add(c2);
    vector<MatchSet> v1 {s1, s2};

    // P2:
    Call c3 = irecv(P2, WILDCARD);
    MatchSet s3;
    s3.add(c3);

    // P2:
    Call c4 = irecv(P2, WILDCARD);
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
