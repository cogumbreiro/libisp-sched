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
 * File:        name2id.cpp
 * Description: Translates from the envelope's string to an enum
 * Contact:     isp-dev@cs.utah.edu
 */

#include "name2id.hpp"


bool is_send(OpType call_type) {
    return (call_type == OpType::SSEND || call_type == OpType::SEND ||
            call_type == OpType::RSEND || call_type == OpType::ISEND);
}

bool is_recv(OpType call_type) {
    return (call_type == OpType::IRECV || call_type == OpType::RECV ||
            call_type == OpType::PROBE || call_type == OpType::IPROBE);
}

bool is_collective(OpType call_type) {
    return call_type == OpType::BARRIER
            || call_type == OpType::BCAST
            || call_type == OpType::CART_CREATE
            || call_type == OpType::COMM_CREATE || call_type == OpType::COMM_DUP
            || call_type == OpType::COMM_SPLIT || call_type == OpType::COMM_FREE
            || call_type == OpType::ALLREDUCE || call_type == OpType::REDUCE
            || call_type == OpType::GATHER || call_type == OpType::SCATTER
            || call_type == OpType::GATHERV || call_type == OpType::SCATTERV
            || call_type == OpType::ALLGATHER || call_type == OpType::ALLGATHERV
            || call_type == OpType::ALLTOALL || call_type == OpType::ALLTOALLV
            || call_type == OpType::SCAN || call_type == OpType::REDUCE_SCATTER;
}

bool is_probe(OpType call_type) {
    return call_type == OpType::PROBE || call_type == OpType::IPROBE;
}

bool is_wait(OpType call_type) {
    return (call_type == OpType::WAIT
            || call_type == OpType::WAITANY
            || call_type == OpType::WAITALL
            );
}

bool is_test(OpType call_type) {
    return (call_type == OpType::TEST
            || call_type == OpType::TESTANY
            || call_type == OpType::TESTALL
            );
}

bool is_blocking(OpType call_type) {
    return call_type == OpType::RECV
            || call_type == OpType::SSEND
            || call_type == OpType::FINALIZE
            || is_probe(call_type)
            || is_test(call_type)
            || is_wait(call_type)
            || is_collective(call_type);
}

std::map<std::string, OpType> name2id::name_id;

bool name2id::init_done = false;

void name2id::doInit () {

    if (init_done) {
        return;
    }

    name_id["MPI_Send"] = OpType::SEND;
    name_id["Send"] = OpType::SEND;

    name_id["MPI_Isend"] = OpType::ISEND;
    name_id["Isend"] = OpType::ISEND;

    name_id["MPI_Ssend"] = OpType::SSEND;
    name_id["Ssend"] = OpType::SSEND;

	name_id["MPI_Rsend"] = OpType::RSEND;
	name_id["Rsend"] = OpType::RSEND;

    name_id["MPI_Send_init"] = OpType::SEND_INIT;
    name_id["Send_init"] = OpType::SEND_INIT;

    name_id["MPI_Recv"] = OpType::RECV;
    name_id["Recv"] = OpType::RECV;

    name_id["MPI_Irecv"] = OpType::IRECV;
    name_id["Irecv"] = OpType::IRECV;

    name_id["MPI_Recv_init"] = OpType::RECV_INIT;
    name_id["Recv_init"] = OpType::RECV_INIT;

    name_id["MPI_Probe"] = OpType::PROBE;
    name_id["Probe"] = OpType::PROBE;

    name_id["MPI_Iprobe"] = OpType::IPROBE;
    name_id["Iprobe"] = OpType::IPROBE;

    name_id["MPI_Start"] = OpType::START;
    name_id["Start"] = OpType::START;

    name_id["MPI_Startall"] = OpType::STARTALL;
    name_id["Startall"] = OpType::STARTALL;

    name_id["MPI_Request_free"] = OpType::REQUEST_FREE;
    name_id["Request_free"] = OpType::REQUEST_FREE;

    name_id["MPI_Wait"] = OpType::WAIT;
    name_id["Wait"] = OpType::WAIT;

    name_id["MPI_Waitall"] = OpType::WAITALL;
    name_id["Waitall"] = OpType::WAITALL;

    name_id["MPI_Waitany"] = OpType::WAITANY;
    name_id["Waitany"] = OpType::WAITANY;

    name_id["MPI_Testany"] = OpType::TESTANY;
    name_id["Testany"] = OpType::TESTANY;

    name_id["MPI_Testall"] = OpType::TESTALL;
    name_id["Testall"] = OpType::TESTALL;

    name_id["MPI_Test"] = OpType::TEST;
    name_id["Test"] = OpType::TEST;

    name_id["MPI_Sendrecv"] = OpType::SENDRECV;
    name_id["Sendrecv"] = OpType::SENDRECV;

    name_id["MPI_Barrier"] = OpType::BARRIER;
    name_id["Barrier"] = OpType::BARRIER;

    name_id["MPI_Bcast"] = OpType::BCAST;
    name_id["Bcast"] = OpType::BCAST;

    name_id["MPI_Scatter"] = OpType::SCATTER;
    name_id["Scatter"] = OpType::SCATTER;

    name_id["MPI_Gather"] = OpType::GATHER;
    name_id["Gather"] = OpType::GATHER;

    name_id["MPI_Scatterv"] = OpType::SCATTERV;
    name_id["Scatterv"] = OpType::SCATTERV;

    name_id["MPI_Gatherv"] = OpType::GATHERV;
    name_id["Gatherv"] = OpType::GATHERV;

    name_id["MPI_Allgather"] = OpType::ALLGATHER;
    name_id["Allgather"] = OpType::ALLGATHER;

    name_id["MPI_AllGatherv"] = OpType::ALLGATHERV;
    name_id["Allgatherv"] = OpType::ALLGATHERV;

    name_id["MPI_Alltoall"] = OpType::ALLTOALL;
    name_id["Alltoall"] = OpType::ALLTOALL;

    name_id["MPI_Alltoallv"] = OpType::ALLTOALLV;
    name_id["Alltoallv"] = OpType::ALLTOALLV;

    name_id["MPI_Scan"] = OpType::SCAN;
    name_id["Scan"] = OpType::SCAN;

    name_id["MPI_Reduce"] = OpType::REDUCE;
    name_id["Reduce"] = OpType::REDUCE;

    name_id["MPI_Reduce_scatter"] = OpType::REDUCE_SCATTER;
    name_id["Reduce_scatter"] = OpType::REDUCE_SCATTER;

    name_id["MPI_Allreduce"] = OpType::ALLREDUCE;
    name_id["Allreduce"] = OpType::ALLREDUCE;

    name_id["MPI_Comm_create"] = OpType::COMM_CREATE;
    name_id["Comm_create"] = OpType::COMM_CREATE;

    name_id["MPI_Cart_create"] = OpType::CART_CREATE;
    name_id["Cart_create"] = OpType::CART_CREATE;

    name_id["MPI_Comm_dup"] = OpType::COMM_DUP;
    name_id["Comm_dup"] = OpType::COMM_DUP;

    name_id["MPI_Comm_split"] = OpType::COMM_SPLIT;
    name_id["Comm_split"] = OpType::COMM_SPLIT;

    name_id["MPI_Comm_free"] = OpType::COMM_FREE;
    name_id["Comm_free"] = OpType::COMM_FREE;

    name_id["MPI_Abort"] = OpType::ABORT;
    name_id["Abort"] = OpType::ABORT;

    name_id["MPI_Finalize"] = OpType::FINALIZE;
    name_id["Finalize"] = OpType::FINALIZE;

    name_id["MPI_Pcontrol"] = OpType::PCONTROL;
    name_id["Pcontrol"] = OpType::PCONTROL;

	name_id["MPI_Exscan"] = OpType::EXSCAN;
	name_id["Exscan"] = OpType::EXSCAN;

    init_done = true;
}

optional<OpType> name2id::getId (std::string name) {
    optional<OpType> result;
    if (! init_done) {
        doInit ();
    }

    auto iter = name_id.find (name);
    if (iter == name_id.end ()) {
        return result;
    }
    result.reset((*iter).second);
    return result;
}
