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

Envelope::Envelope() {
    dest = 0;
    dest_wildcard = false;
    src = 0;
    src_wildcard = false;
}

bool Envelope::operator== (const Envelope &e) const {
    if (e.func != func) {
        return false;
    }
    switch (e.func_id) {
    case OpType::ASSERT:
        return (display_name == e.display_name);

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

    case OpType::LEAK:
        return (e.filename == filename && e.linenumber == linenumber &&
                e.count == count);

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

bool operator<(const Envelope &lhs, const Envelope &rhs) {
    /*
     * Find Intra-Completes-Before :
     * 1) Blocking rule
     */
    if (lhs.isBlockingType()) {
        return true;
    }

    /*
     * 2) Send order rule
     */

    if (lhs.isSendType () &&
        rhs.isSendType () &&
        lhs.dest == rhs.dest &&
        lhs.comm == rhs.comm &&
        lhs.stag == rhs.stag) {
        return true;
    }
    /*
     * 3) Recv order rule
     */
    if (lhs.isRecvType () &&
        rhs.isRecvType () &&
        (lhs.src == rhs.src ||
         lhs.src == WILDCARD) &&
        lhs.comm == rhs.comm &&
        (lhs.rtag == rhs.rtag ||
         lhs.rtag == WILDCARD)) {
        return true;
    }

    /*
     * 4) iRecv -> Wait order rule
     */
    if (lhs.func_id == OpType::IRECV &&
            ((rhs.func_id == OpType::WAIT) ||
             (rhs.func_id == OpType::TEST)) &&
            lhs.count == rhs.count) {
        return true;
    }

    if (lhs.func_id == OpType::ISEND &&
            ((rhs.func_id == OpType::WAIT) ||
             (rhs.func_id == OpType::TEST)) &&
            lhs.count == rhs.count) {
        return true;
    }

    if ((rhs.isWaitType() || rhs.isTestType()) &&
            (lhs.func_id == OpType::IRECV ||
             lhs.func_id == OpType::ISEND)) {

        for (int i = 0 ; i < rhs.count ; i++) {
            if (rhs.req_procs[i] == lhs.index) {
                return true;
            }
        }
    }

    if (rhs.func_id == OpType::FINALIZE) {
        return true;
    }

    return false;
}
