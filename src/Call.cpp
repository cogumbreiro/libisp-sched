#define CATCH_CONFIG_MAIN
#include <catch.hpp>
#include "Call.hpp"

map<int, Process> sort_by_procs(int num_procs, vector<Call> calls) {
    map<int, Process> result;
    for (auto call : calls) {
        result[call.pid].insert(call);
    }
    return result;
}

TEST_CASE("Testing Call", "[call]") {
    vector<Call> calls;
    calls.push_back(Call(1, 0, Envelope()));
    calls.push_back(Call(1, 0, Envelope()));
    calls.push_back(Call(0, 0, Envelope()));
}
