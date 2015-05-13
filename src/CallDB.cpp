#include "CallDB.hpp"

/* Given an evelope return the associated match. */
MPIKind to_kind(const Envelope &env) {
    if (is_collective(env.call_type)) {
        return MPIKind::Collective;
    } else if (is_recv(env.call_type)) {
        return env.src == WILDCARD ? MPIKind::ReceiveAny : MPIKind::Receive;
    } else if (is_send(env.call_type)) {
        return MPIKind::Send;
    } else if (is_wait(env.call_type)) {
        return MPIKind::Wait;
    }
    return MPIKind::Unknown;
}

CallDB::CallDB(const set<Call> & enabled) {
    for (auto call : enabled) {
        add(call);
    }
}

void CallDB::add(Call &call) {
    data[to_kind(call.envelope)].push_back(call);
}

vector<Call> CallDB::at(const MPIKind key) const {
    auto result = data.find(key);
    return (result == data.end()) ? vector<Call>() : result->second;
}

vector<Call> CallDB::findCollective() const {
    // assume all are barrier calls ready to be issued
    return at(MPIKind::Collective);
}

vector<Call> CallDB::findWait() const {
    return at(MPIKind::Wait);
}

vector<Call> CallDB::findReceive() const {
    return at(MPIKind::Receive);
}

vector<Call> CallDB::findReceiveAny() const {
    return at(MPIKind::ReceiveAny);
}

optional<Call> CallDB::matchReceive(const Envelope &recv) const {
    optional<Call> result;
    for (auto send : at(MPIKind::Send)) {
        if (send.envelope.canSend(recv)) {
            result.reset(send);
            break;
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

vector<Call> CallDB::matchReceiveAny(const Envelope &recv) const {
    return get_sends_for(recv, at(MPIKind::Send));
}
