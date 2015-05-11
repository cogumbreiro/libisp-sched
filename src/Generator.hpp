#ifndef _GENERATOR_HPP
#define _GENERATOR_HPP

#include <vector>

#include "Call.hpp"
#include "MatchSet.hpp"
#include "CallDB.hpp"

using std::vector;

/*
 * This class generates a set of batches (MatchSet) of instructions
 * to be run. Each batch should be run in a forked environment.
 */
struct Generator {
    CallDB db;

    Generator(const set<Call> & enabled);

    MatchSet matchCollective() const;

    MatchSet matchReceive() const;

    MatchSet matchWait() const;

    vector<MatchSet> matchReceiveAny() const;

    vector<MatchSet> getMatchSets() const;
};

vector<MatchSet> mix(vector<MatchSet>, vector<MatchSet>);

vector<MatchSet> get_match_sets(set<Call> & enabled);

#endif
