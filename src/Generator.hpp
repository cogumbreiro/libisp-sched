#ifndef _GENERATOR_HPP
#define _GENERATOR_HPP

#include <vector>

#include "Call.hpp"
#include "MatchSet.hpp"

using std::vector;

/* This enumerator is used internally to categorize the calls. */
enum class MPIKind {
    Collective,
    ReceiveAny,
    Receive,
    Send,
    Wait,
    Unknown
};

/* Given an evelope return the associated match. */
MPIKind to_kind(const Envelope &env);

vector<Call> get_sends_for(const Envelope &recv, const vector<Call> &sends);

/*
 * This class generates a set of batches (MatchSet) of instructions
 * to be run. Each batch should be run in a forked environment.
 */
struct Generator {
    Generator(const set<Call> & enabled);

    vector<MatchSet> getMatchSets() const;

    map<MPIKind, vector<Call> > data;

    void add(Call &call);

    vector<Call> at(const MPIKind key) const;

    MatchSet matchCollective() const;

    MatchSet matchReceive() const;

    MatchSet matchWait() const;

    vector<MatchSet> matchReceiveAny() const;
};

vector<MatchSet> mix(vector<MatchSet>, vector<MatchSet>);

vector<MatchSet> get_match_sets(set<Call> & enabled);


#endif
