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
/*
#include <sstream>
#include <iostream>
#include <stdlib.h>
#include <assert.h>

using std::unique_ptr;
using std::string;
*/
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
    case ASSERT:
        return (display_name == e.display_name);

    case BARRIER:
    case BCAST:
    case SCATTER:
    case GATHER:
    case SCATTERV:
    case GATHERV:
    case ALLGATHER:
    case ALLGATHERV:
    case ALLTOALL:
    case ALLTOALLV:
    case SCAN:
	case EXSCAN:
    case ALLREDUCE:
    case REDUCE:
    case REDUCE_SCATTER:
    case CART_CREATE:
    case COMM_CREATE:
    case COMM_DUP:
    case COMM_SPLIT:
    case COMM_FREE:
    case WAIT:
    case TEST:
    case WAITANY:
    case TESTANY:
    case WAITALL:
    case TESTALL:
        return (count == e.count);

    case SEND:
    case SSEND:
    case ISEND:
	case RSEND:
        return (dest==e.dest && e.stag == stag);

    case IRECV:
    case RECV:
    case PROBE:
    case IPROBE:
        return (e.src == src && e.rtag == rtag);

    case ABORT:
    case FINALIZE:
        return true;

    case LEAK:
        return (e.filename == filename && e.linenumber == linenumber &&
                e.count == count);
    case PCONTROL:
        return (e.stag == stag);
    }
    return false;
}

bool Envelope::operator!= (const Envelope &e) const {
    return !((*this) == e);
}
