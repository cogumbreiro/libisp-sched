#include "Generator.hpp"

/* Given an evelope return the associated match. */
MPIKind to_kind(const Envelope &env) {
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

void Generator::add(Call &call) {
    data[to_kind(call.envelope)].push_back(call);
}

vector<Call> Generator::at(const MPIKind key) const {
    auto result = data.find(key);
    return (result == data.end()) ? vector<Call>() : result->second;
}

MatchSet Generator::matchCollective() const {
    // assume all are barrier calls ready to be issued
    return at(MPIKind::Collective);
}

MatchSet Generator::matchReceive() const {
    MatchSet result;
    for (auto recv : at(MPIKind::Receive)) {
        for (auto send : at(MPIKind::Send)) {
            if (send.envelope.canSend(recv.envelope)) {
                result.add(send);
                result.add(recv);
            }
        }
    }
    return result;
}

vector<Call> get_sends_for(const Envelope &recv, vector<Call> &sends) {
    vector<Call> result;
    for (auto send : sends) {
        if (recv.canSend(send.envelope)) {
            result.push_back(send);
        }
    }
    return result;
}

MatchSet Generator::matchWait() const {
    return at(MPIKind::Wait);
}

vector<MatchSet> Generator::matchReceiveAny() const {
    vector<MatchSet> result;
    for (auto recv : at(MPIKind::ReceiveAny)) {
        auto sends = at(MPIKind::Send);
        for (auto send : get_sends_for(recv.envelope, sends)) {
            MatchSet ms;
            recv.envelope.src = send.pid;
            recv.envelope.src_wildcard = false;
            ms.add(recv);
            ms.add(send);
            result.push_back(ms);
        }
    }
    return result;
}

vector<MatchSet> Generator::getMatchSets() const {
    MatchSet ms;
    ms.concat(matchCollective());
    ms.concat(matchReceive());
    ms.concat(matchWait());
    return ms.distribute(matchReceiveAny());
}

vector<MatchSet> get_match_sets(set<Call> & enabled) {
    return Generator(enabled).getMatchSets();
}
