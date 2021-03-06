#include "CallDB.hpp"
#include "Checker.hpp"
#include <cassert>

/* This enumerator is used internally to categorize the calls. */
enum class MPIKind {
    Collective,
    ReceiveAny,
    Receive,
    Send,
    Wait,
    Finalize,
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
    } else if (env.call_type == CallType::FINALIZE) {
        return MPIKind::Finalize;
    }
    return MPIKind::Unknown;
}

CallDB::CallDB(const Schedule &schedule) :
        procs(schedule.procs), participants(schedule.participants) {
    set<Call> calls = set<Call>(schedule.calls.begin(), schedule.calls.end());
    for (auto call : check(calls)) {
        add(call);
    }
}

bool CallDB::add(const Call &call) {
    switch(to_kind(call)) {
    case MPIKind::Collective:
        return addCollective(call);
    case MPIKind::ReceiveAny:
        addReceiveAny(call); return true;
    case MPIKind::Receive:
        addReceive(call); return true;
    case MPIKind::Send:
        addSend(call); return true;
    case MPIKind::Wait:
        addWait(call); return true;
    case MPIKind::Finalize:
        addFinalize(call); return true;
    case MPIKind::Unknown:
        // do nothing
        return false;
    }
    return false;
}

void CallDB::addFinalize(const Call &call) {
    finalize.push_back(call);
    return;
}

vector<Call>& CallDB::getCollective(const Call &call) {
    if (auto comm = call.collective.get(Field::Communicator)) {
        return commCollectives[call.call_type][*comm];
    }
    return worldCollectives[call.call_type];
}

bool CallDB::addCollective(const Call &call) {
    vector<Call> & calls = getCollective(call);
    if (calls.size() > 0 && calls[0].collective != call.collective) {
        // all collectives must have the same parameters to match
        return false;
    }
    calls.push_back(call);
    return true;
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

int CallDB::participantsFor(int comm) const {
    auto iter = participants.find(comm);
    if (iter == participants.end()) {
        return 0;
    }
    return iter->second;
}

vector<Call> CallDB::getCollective(CallType type, int comm) {
    return commCollectives[type][comm];
}

vector<Call> CallDB::getCollective(CallType type) {
    return worldCollectives[type];
}

vector<Call> CallDB::findCollective() const {
    vector<Call> result;
    for (auto & entry : commCollectives) {
        for (auto kv : entry.second) {
            int comm = kv.first;
            vector<Call> & calls = kv.second;
            if (participantsFor(comm) == (int) calls.size()) {
                result.insert(result.end(), calls.begin(), calls.end());
            }
        }
    }
    for (auto & entry : worldCollectives) {
        auto & calls = entry.second;
        if (procs == (int) calls.size()) {
            result.insert(result.end(), calls.begin(), calls.end());
        }
    }
    return result;
}

vector<Call> CallDB::findFinalize() const {
    vector<Call> result;
    if (procs == (int) finalize.size()) {
        result.insert(result.end(), finalize.begin(), finalize.end());
    }
    return result;
}

vector<Call> CallDB::getFinalize() const {
    return finalize;
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
