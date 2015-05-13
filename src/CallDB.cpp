#include "CallDB.hpp"


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
MPIKind to_kind(const Call &env) {
    if (is_collective(env.call_type)) {
        return MPIKind::Collective;
    } else if (is_recv(env.call_type)) {
        return env.recv.src == WILDCARD ? MPIKind::ReceiveAny : MPIKind::Receive;
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

void CallDB::add(const Call &call) {
    switch(to_kind(call)) {
    case MPIKind::Collective:
        addCollective(call); return;
    case MPIKind::ReceiveAny:
        addReceiveAny(call); return;
    case MPIKind::Receive:
        addReceive(call); return;
    case MPIKind::Send:
        addSend(call); return;
    case MPIKind::Wait:
        addWait(call); return;
    case MPIKind::Unknown:
        // do nothing
        return;
    }
}

void CallDB::addCollective(const Call &call) {
    collective.push_back(call);
}

void CallDB::addReceiveAny(const Call &call) {
    receiveAny.push_back(call);
}

void CallDB::addReceive(const Call &call) {
    receive.push_back(call);
}

void CallDB::addSend(const Call &call) {
    send.push_back(call);
}

void CallDB::addWait(const Call &call) {
    wait.push_back(call);
}

vector<Call> CallDB::findCollective() const {
    // assume all are barrier calls ready to be issued
    return collective;
}

vector<Call> CallDB::findWait() const {
    return wait;
}

vector<Call> CallDB::findReceive() const {
    return receive;
}

vector<Call> CallDB::findReceiveAny() const {
    return receiveAny;
}

optional<Call> CallDB::matchReceive(const Call &recv) const {
    optional<Call> result;
    for (auto send : receive) {
        if (send.canSend(recv)) {
            result.reset(send);
            break;
        }
    }
    return result;
}

vector<Call> get_sends_for(const Call &recv, const vector<Call> &sends) {
    vector<Call> result;
    for (auto send : sends) {
        if (send.canSend(recv)) {
            result.push_back(send);
        }
    }
    return result;
}

vector<Call> CallDB::matchReceiveAny(const Call &recv) const {
    return get_sends_for(recv, send);
}
