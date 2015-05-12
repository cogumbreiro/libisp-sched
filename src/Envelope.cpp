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
//    id(0),
//    order_id(0),
//    issue_id(0),
    func_id(OpType::FINALIZE),
    count(0),
    index(0),
    dest(0),
    dest_wildcard(false),
    src(0),
    src_wildcard(false),
    stag(0),
    data_type(0),
    rtag(0)
    {}

Envelope::Envelope(const Envelope &o) {
    func_id = o.func_id;
    count = o.count;
    index = o.index;
    dest = o.dest;
    dest_wildcard = o.dest_wildcard;
    src = o.src;
    src_wildcard = o.src_wildcard;
    stag = o.stag;
    comm = o.comm;
    data_type = o.data_type;
    comm_list = o.comm_list;
    req_procs = o.req_procs;
    rtag = o.rtag;
}

bool Envelope::operator== (const Envelope &e) const {
    switch (e.func_id) {
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
    if (isBlockingType()) {
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
    if (isRecvType () &&
        rhs.isRecvType () &&
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
    if (func_id == OpType::IRECV &&
            ((rhs.func_id == OpType::WAIT) ||
             (rhs.func_id == OpType::TEST)) &&
            count == rhs.count) {
        return true;
    }

    if (func_id == OpType::ISEND &&
            ((rhs.func_id == OpType::WAIT) ||
             (rhs.func_id == OpType::TEST)) &&
            count == rhs.count) {
        return true;
    }

    if ((rhs.isWaitType() || rhs.isTestType()) &&
            (func_id == OpType::IRECV ||
            func_id == OpType::ISEND) && rhs.requested(index)) {
        return true;
    }

    if (rhs.func_id == OpType::FINALIZE) {
        return true;
    }

    return false;
}

bool Envelope::isSendType () const {
    return (func_id == OpType::SSEND || func_id == OpType::SEND ||
            func_id == OpType::RSEND || func_id == OpType::ISEND);
}

bool Envelope::isRecvType () const {
    return (func_id == OpType::IRECV || func_id == OpType::RECV ||
            func_id == OpType::PROBE || func_id == OpType::IPROBE);
}

bool Envelope::isCollectiveType () const {
    return (func_id == OpType::BARRIER || func_id == OpType::BCAST
            || func_id == OpType::CART_CREATE
            || func_id == OpType::COMM_CREATE || func_id == OpType::COMM_DUP
            || func_id == OpType::COMM_SPLIT || func_id == OpType::COMM_FREE
            || func_id == OpType::ALLREDUCE || func_id == OpType::REDUCE
            || func_id == OpType::GATHER || func_id == OpType::SCATTER
            || func_id == OpType::GATHERV || func_id == OpType::SCATTERV
            || func_id == OpType::ALLGATHER || func_id == OpType::ALLGATHERV
            || func_id == OpType::ALLTOALL || func_id == OpType::ALLTOALLV
            || func_id == OpType::SCAN || func_id == OpType::REDUCE_SCATTER);
}

bool Envelope::isBlockingType() const {
    return (func_id == OpType::RECV || func_id == OpType::SSEND
            || isProbeType()
            || func_id == OpType::FINALIZE
            || isTestType()
            || isWaitType()
            || isCollectiveType());
}

bool Envelope::isProbeType() const {
    return func_id == OpType::PROBE || func_id == OpType::IPROBE;
}

bool Envelope::isWaitType() const {
    return (func_id == OpType::WAIT
            || func_id == OpType::WAITANY
            || func_id == OpType::WAITALL
            );
}

bool Envelope::isTestType() const {
    return (func_id == OpType::TEST
            || func_id == OpType::TESTANY
            || func_id == OpType::TESTALL
            );
}

bool Envelope::matchRecv(const Envelope & other) const {
    return isRecvType() &&
        other.isRecvType() &&
        src == other.src &&
        comm == other.comm &&
        rtag == other.rtag;
}

bool Envelope::matchSend(const Envelope & other) const {
    return isSendType() && other.isSendType() &&
        dest == other.dest &&
        comm == other.comm &&
        stag == other.stag;
}

bool Envelope::canSend(const Envelope & recv) const {
    return isSendType() && recv.isRecvType() &&
        comm == recv.comm &&
        (dest == recv.src || recv.src == WILDCARD) &&
        (stag == recv.rtag || recv.rtag == WILDCARD);
}

bool Envelope::requested(int index) const {
    return req_procs.find(index) != req_procs.end();
}
