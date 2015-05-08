#ifndef _MATCH_MAP_HPP
#define _MATCH_MAP_HPP

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

struct MatchMap {
    MatchMap(const set<Call> & enabled) {
        for (auto call : enabled) {
            add(call);
        }
    }

    vector<MatchSet> getMatchSets() const;

private:
    map<MPIKind, vector<Call> > data;

    void add(Call &call);

    vector<Call> at(const MPIKind key) const;

    MatchSet matchCollective() const;

    MatchSet matchReceive() const;

    MatchSet matchWait() const;

    vector<MatchSet> matchReceiveAny() const;
};

vector<MatchSet> get_match_sets(set<Call> & enabled);


#endif
