#include "Checker.hpp"

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

set<Call> check(const set<Call> calls) {
    set<Call> result;
    for (auto proc : sort_by_procs(calls)) {
        auto enabled = filter_enabled(proc.second);
        result.insert(enabled.begin(), enabled.end());
    }
    return result;
}
