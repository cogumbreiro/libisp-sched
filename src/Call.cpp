#include "Call.hpp"

Call::Call() : pid(0), handle(0), call_type(OpType::FINALIZE) {}

Call::Call(int p, int i): pid(p), handle(i), call_type(OpType::FINALIZE),
recv(), send(), wait() {}

Call::Call(const Call & c):
    pid(c.pid),
    handle(c.handle),
    call_type(c.call_type),
    recv(c.recv),
    send(c.send),
    wait(c.wait) {}

bool Call::completesBefore(const Call & rhs) const {
    if (pid != rhs.pid || handle >= rhs.handle) {
        return false;
    }
    /*
     * Find Intra-Completes-Before :
     * 1) Blocking rule
     */
    if (is_blocking(call_type)) {
        return true;
    }

    /*
     * 2) Send order rule
     */
    if (is_send(call_type) && is_send(rhs.call_type)
            && send.completesBefore(rhs.send)) {
        return true;
    }

    /*
     * 3) Recv order rule
     */
    if (is_recv(call_type) &&
        is_recv(rhs.call_type) &&
        recv.completesBefore(rhs.recv)) {
        return true;
    }

    /*
     * 4) iRecv -> Wait order rule
     */
    if (call_type == OpType::IRECV &&
            ((rhs.call_type == OpType::WAIT) ||
             (rhs.call_type == OpType::TEST))) {
        return true;
    }

    if ((call_type == OpType::IRECV || call_type == OpType::ISEND)
            && (is_wait(rhs.call_type) || is_test(rhs.call_type))
            && rhs.wait.requested(handle)) {
        return true;
    }

    if (rhs.call_type == OpType::FINALIZE) {
        return true;
    }

    return false;
}

bool Call::hasAncestors(const set<Call> & calls) const {
    for (auto other : calls) {
        assert(pid == other.pid);
        if (other.handle >= handle) {
            return false;
        }
        if (other.completesBefore(*this)) {
            return true;
        }
    }
    return false;
}

bool Call::operator== (const Call &c) const {
    return pid == c.pid
        && handle == c.handle
        && call_type == c.call_type
        && send == c.send
        && recv == c.recv
        && wait == c.wait;
}

bool Call::operator!= (const Call &c) const {
    return !(*this == c);
}

bool Call::canSend(const Call & recv) const {
    return is_send(call_type) && is_recv(recv.call_type) &&
        send.canSend(recv.recv);
}
