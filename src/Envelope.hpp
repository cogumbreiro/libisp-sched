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
#include <set>

#include "name2id.hpp"

using std::string;

#define WILDCARD (-1)

struct Envelope {
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
    std::set<int> req_procs;

    int                 rtag;
    int                 nprocs;
    int                 comm_split_color;
    int                 comm_id;
    string         filename;
    int                 linenumber;
    unsigned int        ref;
    bool in_exall;

    Envelope();

    Envelope(const Envelope &o);

    bool operator==(const Envelope &) const;

    bool operator!=(const Envelope &) const;

    void issue();

    bool isSendType () const;

    bool isRecvType () const;

    bool isCollectiveType () const;

    bool isBlockingType() const;

    bool isProbeType() const;

    bool isWaitType() const;

    bool isTestType() const;

    bool matchRecv(const Envelope & other) const;

    bool matchSend(const Envelope & other) const;

    bool canSend(const Envelope & recv) const;

    bool requested(int index) const;

    /**
     * Defines the Intra-CB relation
     */
     bool completesBefore(Envelope const &) const;
};

#endif
