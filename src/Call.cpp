#include "Call.hpp"

Call::Call() : pid(0), handle(0), call_type(CallType::FINALIZE) {}

Call::Call(const Call & c):
    pid(c.pid),
    handle(c.handle),
    call_type(c.call_type),
    recv(c.recv),
    send(c.send),
    wait(c.wait),
    collective(c.collective) {}

bool Call::completesBefore(const Call & rhs) const {
    /*
     * Copyright (c) 2008-2009
     *
     * Copyright (c) 2015 Tiago Cogumbreiro
     *
     * School of Computing, University of Utah,
     * Salt Lake City, UT 84112, USA
     *
     * and the Gauss Group
     * http://www.cs.utah.edu/formal_verification
     *
     * See LICENSE.ISP for licensing information
     */
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
    if (is_send(call_type) && is_send(rhs.call_type) && send == rhs.send) {
        return true;
    }

    /*
     * 3) Recv order rule
     */
    if (is_recv(call_type) && is_recv(rhs.call_type)
            && recv.matches(rhs.recv)) {
        return true;
    }

    /*
     * 4) iRecv -> Wait order rule
     */
    if (call_type == CallType::IRECV &&
            ((rhs.call_type == CallType::WAIT) ||
             (rhs.call_type == CallType::TEST))) {
        return true;
    }

    if ((call_type == CallType::IRECV || call_type == CallType::ISEND)
            && (is_wait(rhs.call_type) || is_test(rhs.call_type))
            && rhs.wait.requested(handle)) {
        return true;
    }

    if (rhs.call_type == CallType::FINALIZE) {
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
    return is_send(call_type) && is_recv(recv.call_type)
            && send.canSend(recv.recv);
}
