#ifndef _MATCH_SET_HPP
#define _MATCH_SET_HPP

#include <vector>

#include "Call.hpp"

struct MatchSet {
    MatchSet() {}

    MatchSet(vector<Call> calls) : data(calls) {}

    void add(const Call &call);

    void append(const MatchSet &other);

    void prepend(const MatchSet &other);

    vector<Call> toVector() const;

    bool empty() const;
private:
    vector<Call> data;
};

#endif
