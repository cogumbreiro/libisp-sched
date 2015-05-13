/*
 * Copyright (c) 2008-2009
 *
 * School of Computing, University of Utah,
 * Salt Lake City, UT 84112, USA
 *
 * and the Gauss Group
 * http://www.cs.utah.edu/formal_verification
 *
 * See LICENSE for licensing information
 */

#include "Envelope.hpp"

Envelope::Envelope() :
    pid(0),
    call_type(OpType::FINALIZE),
    count(0),
    src(0),
    dest(0),
    stag(0),
    rtag(0)
    {}

Envelope::Envelope(const Envelope &o) :
    pid(o.pid),
    call_type(o.call_type),
    count(o.count),
    src(o.src),
    dest(o.dest),
    stag(o.stag),
    rtag(o.rtag),
    requests(o.requests),
    comm(o.comm)
    {}

bool Envelope::operator== (const Envelope &e) const {
    return pid == e.pid
            && call_type == e.call_type
            && count == e.count
            && src == e.src
            && dest == e.dest
            && stag == e.stag
            && rtag == e.rtag
            && requests == e.requests
            && comm == e.comm;
}

bool Envelope::operator!= (const Envelope &e) const {
    return !((*this) == e);
}

bool Envelope::completesBefore(const Envelope &rhs) const {
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
            && src == rhs.src
            && (stag == WILDCARD || stag == rhs.stag)
            && comm == rhs.comm) {
        return true;
    }

    /*
     * 3) Recv order rule
     */
    if (is_recv(call_type) &&
        is_recv(rhs.call_type) &&
        (src == rhs.src ||
         src == WILDCARD) &&
        comm == rhs.comm &&
        (rtag == rhs.rtag ||
         rtag == WILDCARD)) {
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
            && rhs.requests.find(handle) != rhs.requests.end()) {
        return true;
    }

    if (rhs.call_type == OpType::FINALIZE) {
        return true;
    }

    return false;
}

bool Envelope::canSend(const Envelope & recv) const {
    return is_send(call_type) && is_recv(recv.call_type) &&
        comm == recv.comm &&
        count == recv.count &&
        (dest == recv.src || recv.src == WILDCARD) &&
        (stag == recv.rtag || recv.rtag == WILDCARD);
}
/*
bool Envelope::requested(int index) const {
    return requests.find(index) != requests.end();
}*/
