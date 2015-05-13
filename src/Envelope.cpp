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
    switch (e.call_type) {
    case OpType::BARRIER:
    case OpType::BCAST:
    case OpType::SCATTER:
    case OpType::GATHER:
    case OpType::SCATTERV:
    case OpType::GATHERV:
    case OpType::ALLGATHER:
    case OpType::ALLGATHERV:
    case OpType::ALLTOALL:
    case OpType::ALLTOALLV:
    case OpType::SCAN:
	case OpType::EXSCAN:
    case OpType::ALLREDUCE:
    case OpType::REDUCE:
    case OpType::REDUCE_SCATTER:
    case OpType::CART_CREATE:
    case OpType::COMM_CREATE:
    case OpType::COMM_DUP:
    case OpType::COMM_SPLIT:
    case OpType::COMM_FREE:
    case OpType::WAIT:
    case OpType::TEST:
    case OpType::WAITANY:
    case OpType::TESTANY:
    case OpType::WAITALL:
    case OpType::TESTALL:
    case OpType::STARTALL:
        return (count == e.count);

    case OpType::SEND:
    case OpType::SSEND:
    case OpType::ISEND:
	case OpType::RSEND:
    case OpType::SEND_INIT:
        return (dest==e.dest && e.stag == stag);

    case OpType::IRECV:
    case OpType::RECV:
    case OpType::PROBE:
    case OpType::IPROBE:
    case OpType::RECV_INIT:
        return (e.src == src && e.rtag == rtag);

    case OpType::PCONTROL:
        return (e.stag == stag);

    case OpType::SENDRECV:
        return (e.src == src && dest == e.dest
                && stag == e.stag && rtag == e.rtag);

    case OpType::START:
    case OpType::ABORT:
    case OpType::FINALIZE:
    case OpType::REQUEST_FREE:
        return true;

    }
    return false;
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
    if (matchSend(rhs)) {
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
             (rhs.call_type == OpType::TEST)) &&
            count == rhs.count) {
        return true;
    }

    if (call_type == OpType::ISEND &&
            ((rhs.call_type == OpType::WAIT) ||
             (rhs.call_type == OpType::TEST)) &&
            count == rhs.count) {
        return true;
    }

    if ((is_wait(rhs.call_type) || is_test(rhs.call_type)) &&
            (call_type == OpType::IRECV ||
            call_type == OpType::ISEND) && rhs.requested(handle)) {
        return true;
    }

    if (rhs.call_type == OpType::FINALIZE) {
        return true;
    }

    return false;
}
/*
bool Envelope::isSendType () const {
    return (call_type == OpType::SSEND || call_type == OpType::SEND ||
            call_type == OpType::RSEND || call_type == OpType::ISEND);
}

bool Envelope::isRecvType () const {
    return (call_type == OpType::IRECV || call_type == OpType::RECV ||
            call_type == OpType::PROBE || call_type == OpType::IPROBE);
}

bool Envelope::isCollectiveType () const {
    return (call_type == OpType::BARRIER || call_type == OpType::BCAST
            || call_type == OpType::CART_CREATE
            || call_type == OpType::COMM_CREATE || call_type == OpType::COMM_DUP
            || call_type == OpType::COMM_SPLIT || call_type == OpType::COMM_FREE
            || call_type == OpType::ALLREDUCE || call_type == OpType::REDUCE
            || call_type == OpType::GATHER || call_type == OpType::SCATTER
            || call_type == OpType::GATHERV || call_type == OpType::SCATTERV
            || call_type == OpType::ALLGATHER || call_type == OpType::ALLGATHERV
            || call_type == OpType::ALLTOALL || call_type == OpType::ALLTOALLV
            || call_type == OpType::SCAN || call_type == OpType::REDUCE_SCATTER);
}

bool Envelope::isBlockingType() const {
    return (call_type == OpType::RECV || call_type == OpType::SSEND
            || isProbeType()
            || call_type == OpType::FINALIZE
            || isTestType()
            || isWaitType()
            || isCollectiveType());
}

bool Envelope::isProbeType() const {
    return call_type == OpType::PROBE || call_type == OpType::IPROBE;
}

bool Envelope::isWaitType() const {
    return (call_type == OpType::WAIT
            || call_type == OpType::WAITANY
            || call_type == OpType::WAITALL
            );
}

bool Envelope::isTestType() const {
    return (call_type == OpType::TEST
            || call_type == OpType::TESTANY
            || call_type == OpType::TESTALL
            );
}
*/

bool Envelope::matchRecv(const Envelope & other) const {
    return is_recv(call_type) &&
        is_recv(other.call_type) &&
        src == other.src &&
        comm == other.comm &&
        rtag == other.rtag;
}

bool Envelope::matchSend(const Envelope & other) const {
    return is_send(call_type) && is_send(other.call_type) &&
        dest == other.dest &&
        comm == other.comm &&
        stag == other.stag;
}

bool Envelope::canSend(const Envelope & recv) const {
    return is_send(call_type) && is_recv(recv.call_type) &&
        comm == recv.comm &&
        (dest == recv.src || recv.src == WILDCARD) &&
        (stag == recv.rtag || recv.rtag == WILDCARD);
}

bool Envelope::requested(int index) const {
    return requests.find(index) != requests.end();
}
