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


bool is_send(OpType call_type);

bool is_recv(OpType call_type);

bool is_collective(OpType call_type);

bool is_probe(OpType call_type);

bool is_wait(OpType call_type);

bool is_test(OpType call_type);

bool is_blocking(OpType call_type);

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
