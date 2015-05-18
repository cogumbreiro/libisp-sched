#define CATCH_CONFIG_MAIN
#include <catch.hpp>

#include "Call.hpp"
#include "Process.hpp"

Call call(int pid, int handle) {
    Call c;
    c.pid = pid;
    c.handle = handle;
    return c;
}

Call isend(Process &p, int pid) {
    return p.isend(0, 0, pid, 0, 0);
}

TEST_CASE("Testing call::equals") {
    REQUIRE(call(1, 10) == call(1, 10));
    REQUIRE(call(1, 5) == call(1, 5));
    REQUIRE(call(0, 2) == call(0, 2));
}

TEST_CASE("Testing call::neq") {
    REQUIRE(call(1, 10) != call(1, 0));
    REQUIRE(call(1, 5) != call(2, 5));
}

TEST_CASE("Testing call::<") {
    REQUIRE(!(call(1, 10) < call(1, 10)));
    REQUIRE(!(call(1, 10) < call(1, 5)));
    REQUIRE(!(call(1, 10) < call(0, 2)));


    REQUIRE(!(call(1, 50) < call(1, 10)));
    REQUIRE(!(call(1, 5) < call(1, 5)));
    REQUIRE(!(call(1, 5) < call(0, 2)));

    REQUIRE((call(0, 2) < call(1, 10)));
    REQUIRE((call(0, 2) < call(1, 5)));
    REQUIRE(!(call(0, 2) < call(0, 2)));
}

TEST_CASE("regression-1") {
    auto P0 = Process(0), P1 = Process(1);
    Call c1 = isend(P0, P1.pid);
    Call c3 = P0.wait(c1.handle);
    REQUIRE(c1.call_type == CallType::ISEND);
    REQUIRE(c3.call_type == CallType::WAIT);
    REQUIRE(c1.pid == c3.pid);
    REQUIRE(c1.handle == 0);
    REQUIRE(c3.handle == 1);
    REQUIRE(c1 < c3);
    REQUIRE(c1.completesBefore(c3));
}

TEST_CASE("regression-2") {
    auto P0 = Process(0), P1 = Process(1);
    // GET THE SECOND PHASE ONCE ALL PROCESSES ARE BLOCKED
    set<Call> trace;
    // P0:
    Call c1 = isend(P0, P1.pid);
    trace.insert(c1);
    Call c3 = P0.wait(c1.handle);
    REQUIRE(c1.pid == c3.pid);
    REQUIRE(c1.handle < c3.handle);
    REQUIRE(c1.completesBefore(c3));
    REQUIRE(c3.hasAncestors(trace));
}

TEST_CASE("regression-3") {
    auto P0 = Process(0);
    auto c = P0.barrier(10);
    auto comm = c.collective.get(Field::Communicator);
    REQUIRE(c.pid == 0);
    REQUIRE(c.handle == 0);
    REQUIRE(comm);
    REQUIRE(*comm == 10);
    REQUIRE(is_collective(c.call_type));
}
