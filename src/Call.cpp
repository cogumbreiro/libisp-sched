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

multimap<int, Call> sort_by_procs(const set<Call> & calls) {
    multimap<int, Call> result;
    for (auto call : calls) {
        result.insert(pair<int,Call>(call.pid, call));
    }
    return result;
}
/*
set<Call> filter_enabled(const set<Call> & calls) {

}*/


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
    auto range = procs.equal_range(0);
    auto it = range.first;
    REQUIRE(it != range.second);
    REQUIRE(it->second == Call(0, 2, Envelope()));
    it++;
    REQUIRE(it == range.second);

    range = procs.equal_range(1);
    it = range.first;
    REQUIRE(it != range.second);
    REQUIRE(it->second == Call(1, 5, Envelope()));
    it++;
    REQUIRE(it != range.second);
    REQUIRE(it->second == Call(1, 10, Envelope()));
    it++;
    REQUIRE(it == range.second);
}
