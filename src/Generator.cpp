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

Generator::Generator(const set<Call> & enabled) {
    for (auto call : enabled) {
        add(call);
    }
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

vector<Call> get_sends_for(const Envelope &recv, const vector<Call> &sends) {
    vector<Call> result;
    for (auto send : sends) {
        if (send.envelope.canSend(recv)) {
            result.push_back(send);
        }
    }
    return result;
}

MatchSet Generator::matchWait() const {
    return at(MPIKind::Wait);
}

vector<MatchSet> add_prefix(const MatchSet match, const vector<MatchSet> matches) {
    if (match.empty()) {
        return matches;
    }
    if (matches.empty()) {
        vector<MatchSet> result;
        result.push_back(match);
        return result;
    }
    vector<MatchSet> result;
    for (auto other : matches) {
        other.prepend(match);
        result.push_back(other);
    }
    return result;
}

vector<MatchSet> mix(vector<MatchSet> left, vector<MatchSet> right) {
    if (left.empty()) {
        return right;
    }
    if (right.empty()) {
        return left;
    }
    vector<MatchSet> result;
    for (auto elem : left) {
        auto prefixed = std::move(add_prefix(elem, right));
        result.insert(result.end(), prefixed.begin(), prefixed.end());
    }
    return result;
}

vector<MatchSet> Generator::matchReceiveAny() const {
    vector<MatchSet> result;
    for (auto recv : at(MPIKind::ReceiveAny)) {
        vector<MatchSet> receive;
        auto sends = at(MPIKind::Send);
        for (auto send : get_sends_for(recv.envelope, sends)) {
            MatchSet ms;
            recv.envelope.src = send.pid;
            recv.envelope.src_wildcard = false;
            ms.add(recv);
            ms.add(send);
            receive.push_back(ms);
        }
        result = mix(receive, result);
    }
    return result;
}

vector<MatchSet> Generator::getMatchSets() const {
    MatchSet ms;
    ms.append(std::move(matchCollective()));
    ms.append(std::move(matchReceive()));
    ms.append(std::move(matchWait()));
    vector<MatchSet> result;
    result.push_back(ms);
    return ms.empty() ? matchReceiveAny() : result;
}

vector<MatchSet> get_match_sets(set<Call> & enabled) {
    return Generator(enabled).getMatchSets();
}
