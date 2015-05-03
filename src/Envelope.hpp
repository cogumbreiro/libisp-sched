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
 * File:        Envelope.hpp
 * Description: Implements the various envelopes received from profiled code
 * Contact:     isp-dev@cs.utah.edu
 */

#ifndef _ENVELOPE_HPP
#define _ENVELOPE_HPP

#include <vector>
#include "name2id.hpp"

using std::string;

#define WILDCARD (-1)

class Envelope {
public:
    /*
     * The envelope contains all the possible parameters for a
     * function. Only those valid for a particular function must
     * be used. Use with care!
     */
    int                 id;
    int                 order_id;
    int                 issue_id;
    string         func;
    string         display_name;
    OpType                 func_id;
    int                 count;
    int                 index;
    int                 dest;
    bool                dest_wildcard;
    int                 src;
    bool                src_wildcard;
    int                 stag;
    string         comm;
    int					data_type; //CGD
    std::vector<int>    comm_list;
    std::vector<int>    req_procs;

    int                 rtag;
    int                 nprocs;
    int                 comm_split_color;
    int                 comm_id;
    string         filename;
    int                 linenumber;
    unsigned int        ref;
    bool in_exall;

    bool operator==(const Envelope &) const;

    bool operator!=(const Envelope &) const;

    Envelope();

    Envelope(const Envelope &e);

    inline void issue() {
        static int issue_number = 0;
        issue_id = ++issue_number;
    }

    inline bool isSendType () const {
        return (func_id == OpType::SSEND || func_id == OpType::SEND ||
				func_id == OpType::RSEND || func_id == OpType::ISEND);
    }

    inline bool isRecvType () const {
        return (func_id == OpType::IRECV || func_id == OpType::RECV ||
                func_id == OpType::PROBE || func_id == OpType::IPROBE);
    }

    inline bool isCollectiveType () const {
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

    inline bool isBlockingType() const {
        return (func_id == OpType::RECV || func_id == OpType::SSEND
                || isProbeType()
                || func_id == OpType::FINALIZE
                || isTestType()
                || isWaitType()
                || isCollectiveType());
    }

    inline bool isProbeType() const {
        return func_id == OpType::PROBE || func_id == OpType::IPROBE;
    }

    inline bool isWaitType() const {
        return (func_id == OpType::WAIT
                || func_id == OpType::WAITANY
                || func_id == OpType::WAITALL
                );
    }

    inline bool isTestType() const {
        return (func_id == OpType::TEST
                || func_id == OpType::TESTANY
                || func_id == OpType::TESTALL
                );
    }

    inline bool matchRecv(const Envelope & other) const {
        return isRecvType() &&
            other.isRecvType() &&
            src == other.src &&
            comm == other.comm &&
            rtag == other.rtag;
    }

    inline bool matchSend(const Envelope & other) const {
        return isSendType() && other.isSendType() &&
            dest == other.dest &&
            comm == other.comm &&
            stag == other.stag;
    }

    inline bool canSend(const Envelope & recv) const {
        return isSendType() && recv.isRecvType() &&
            comm == recv.comm &&
            dest == recv.src &&
            (stag == recv.rtag || recv.rtag == WILDCARD);
    }
    /**
     * Defines the Intra-CB relation
     */
    friend bool operator<(const Envelope &lhs, const Envelope &rhs);
};

#endif
