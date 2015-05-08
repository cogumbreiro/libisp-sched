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
MPIKind get_kind(const Envelope &env) {
    if (env.isCollectiveType()) {
        return MPIKind::Collective;
    } else if (env.isRecvType()) {
        return env.src == WILDCARD ? MPIKind::ReceiveAny : MPIKind::Receive;
    } else if (env.isSendType()) {
        return MPIKind::Send;
    } else if (env.isWaitType()) {
        return MPIKind::Wait;
    }
    return MPIKind::Unknown;
}

struct MatchMap {
    MatchMap(const set<Call> & enabled) {
        for (auto call : enabled) {
            add(call);
        }
    }

    vector<MatchSet> getMatchSets() const;

    static vector<MatchSet> get_match_sets(set<Call> & enabled);

private:
    map<MPIKind, vector<Call> > data;

    void add(Call &call);

    vector<Call> at(const MPIKind key) const;

    MatchSet matchCollective() const;

    MatchSet matchReceive() const;

    MatchSet matchWait() const;

    vector<MatchSet> matchReceiveAny() const;
};



#endif
