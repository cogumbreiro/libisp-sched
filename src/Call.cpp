#define CATCH_CONFIG_MAIN
#include <catch.hpp>
#include "Call.hpp"
#include <map>
using std::multimap;
using std::pair;

bool Call::completesBefore(const Call & call) const {
    return pid == call.pid && index < call.index
            && envelope.completesBefore(call.envelope);
}

bool Call::hasAncestors(const set<Call> & calls) const {
    for (auto other : calls) {
        assert(pid == other.pid);
        if (index >= other.index) {
            return false;
        }
        if (other.completesBefore(*this)) {
            return true;
        }
    }
    return false;
}

map<int, set<Call> > sort_by_procs(const set<Call> & calls) {
    map<int, set<Call> > result;
    for (auto call : calls) {
        result[call.pid].insert(call);
    }
    return result;
}

set<Call> filter_enabled(const set<Call> & calls) {
    set<Call> result;
    for (auto c : calls) {
        if (! c.hasAncestors(calls)) {
            result.insert(c);
        }
    }
    return result;
}


TEST_CASE("Testing Call::equals", "[call]") {
    REQUIRE(Call(1, 10, Envelope()) == Call(1, 10, Envelope()));
    REQUIRE(Call(1, 5, Envelope()) == Call(1, 5, Envelope()));
    REQUIRE(Call(0, 2, Envelope()) == Call(0, 2, Envelope()));
    // neq
    REQUIRE(Call(1, 10, Envelope()) != Call(1, 0, Envelope()));
    REQUIRE(Call(1, 5, Envelope()) != Call(2, 5, Envelope()));
}

TEST_CASE("Testing Call::lt", "[call]") {
    REQUIRE(!(Call(1, 10, Envelope()) < Call(1, 10, Envelope())));
    REQUIRE(!(Call(1, 10, Envelope()) < Call(1, 5, Envelope())));
    REQUIRE(!(Call(1, 10, Envelope()) < Call(0, 2, Envelope())));


    REQUIRE((Call(1, 5, Envelope()) < Call(1, 10, Envelope())));
    REQUIRE(!(Call(1, 5, Envelope()) < Call(1, 5, Envelope())));
    REQUIRE(!(Call(1, 5, Envelope()) < Call(0, 2, Envelope())));

    REQUIRE((Call(0, 2, Envelope()) < Call(1, 10, Envelope())));
    REQUIRE((Call(0, 2, Envelope()) < Call(1, 5, Envelope())));
    REQUIRE(!(Call(0, 2, Envelope()) < Call(0, 2, Envelope())));
}

TEST_CASE("Testing sort_by_procs", "[call]") {
    set<Call> calls;
    calls.insert(Call(1, 10, Envelope()));
    calls.insert(Call(1, 5, Envelope()));
    calls.insert(Call(0, 2, Envelope()));

    auto procs = sort_by_procs(calls);
    auto proc1 = procs[0];
    auto it = proc1.begin();
    REQUIRE(it != proc1.end());
    REQUIRE((*it) == Call(0, 2, Envelope()));
    it++;
    REQUIRE(it == proc1.end());

    auto proc2 = procs[1];
    it = proc2.begin();
    REQUIRE(it != proc2.end());
    REQUIRE((*it) == Call(1, 5, Envelope()));
    it++;
    REQUIRE(it != proc2.end());
    REQUIRE((*it) == Call(1, 10, Envelope()));
    it++;
    REQUIRE(it == proc2.end());

    auto proc3 = procs[10];
    REQUIRE(proc3.size() == 0);
}

// DOI: 10.1007/978-3-642-03770-2_33
TEST_CASE("ISP Tool Update: Scalable MPI Verification", "example-1") {
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
    Call c3(P0, 2, Envelope::Wait(0));
    //trace.insert(c3);
    REQUIRE(! c1.completesBefore(c2));
    REQUIRE(! c2.completesBefore(c1));

    // P1:
    Call c4(P1, 0, Envelope::IRecv(WILDCARD));
    trace.insert(c4);
    Call c5(P1, 1, Envelope::Barrier());
    trace.insert(c5);
    Call c6(P1, 2, Envelope::Wait(0));
    //trace.insert(c6);
    REQUIRE(! c4.completesBefore(c5));
    REQUIRE(! c5.completesBefore(c4));
    // P2:
    Call c7(P2, 0, Envelope::Barrier());
    trace.insert(c7);
    Call c8(P2, 1, Envelope::ISend(P1));
    //trace.insert(c8);
    Call c9(P2, 2, Envelope::Wait(1));
    //trace.insert(c9);

    auto procs = sort_by_procs(trace);
    set<Call> proc0 = procs[P0];
    proc0 = filter_enabled(proc0);
    REQUIRE(2 == proc0.size());
    REQUIRE(proc0.find(c1) != proc0.end());
    REQUIRE(proc0.find(c2) != proc0.end());
}
