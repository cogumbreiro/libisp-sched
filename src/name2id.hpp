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

/*
 * ISP: MPI Dynamic Verification Tool
 *
 * File:        name2id.hpp
 * Description: Translates from the envelope's string to an enum
 * Contact:     isp-dev@cs.utah.edu
 */

#ifndef _NAME_2_ID_HPP
#define _NAME_2_ID_HPP

#include <map>
#include <string>
#include <boost/optional.hpp>

using boost::optional;

enum class OpType {
    SEND,
	RSEND,
    SSEND,
    SEND_INIT,
    RECV,
    PROBE,
    IPROBE,
    SENDRECV,
    RECV_INIT,
    BARRIER,
    BCAST,
    SCATTER,
    GATHER,
    SCATTERV,
    GATHERV,
    ALLGATHER,
    ALLGATHERV,
    ALLTOALL,
    ALLTOALLV,
    SCAN,
    REDUCE,
    REDUCE_SCATTER,
    ISEND,
    IRECV,
    START,
    STARTALL,
    WAIT,
    TEST,
    ALLREDUCE,
    WAITALL,
    TESTALL,
    WAITANY,
    TESTANY,
    REQUEST_FREE,
    CART_CREATE,
    COMM_CREATE,
    COMM_DUP,
    COMM_SPLIT,
    COMM_FREE,
    ABORT,
    FINALIZE,
    PCONTROL,
	EXSCAN
};


bool is_send(OpType call_type) const {
    return (call_type == OpType::SSEND || call_type == OpType::SEND ||
            call_type == OpType::RSEND || call_type == OpType::ISEND);
}

bool is_recv(OpType call_type) const {
    return (call_type == OpType::IRECV || call_type == OpType::RECV ||
            call_type == OpType::PROBE || call_type == OpType::IPROBE);
}

bool is_collective(OpType call_type) const {
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

bool is_blocking(OpType call_type) const {
    return (call_type == OpType::RECV
            || call_type == OpType::SSEND
            || call_type == OpType::FINALIZE
            || isProbeType()
            || isTestType()
            || isWaitType()
            || isCollectiveType());
}

bool is_probe(OpType call_type) const {
    return call_type == OpType::PROBE || call_type == OpType::IPROBE;
}

bool is_wait(OpType call_type) const {
    return (call_type == OpType::WAIT
            || call_type == OpType::WAITANY
            || call_type == OpType::WAITALL
            );
}

bool is_test(OpType call_type) const {
    return (call_type == OpType::TEST
            || call_type == OpType::TESTANY
            || call_type == OpType::TESTALL
            );
}

class name2id {


public:
    /*
     * The names are MPI function names.
     * Names cam be of the form "MPI_Send" or "Send"
     */
    static optional<OpType> getId (std::string name);

private:
    static void doInit ();

    static std::map <std::string, OpType> name_id;
    static bool init_done;

    /*
     * Disallow objects of this class
     */
    name2id ();
    name2id (name2id &);
    name2id& operator= (name2id &);


};

#endif
