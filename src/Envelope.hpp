/*
 * Copyright (c) 2008-2009
 *
 * Copyright (c) 2015 Tiago Cogumbreiro
 *
 * School of Computing, University of Utah,
 * Salt Lake City, UT 84112, USA
 *
 * and the Gauss Group
 * http://www.cs.utah.edu/formal_verification
 *
 * See LICENSE for licensing information
 */

#ifndef _ENVELOPE_HPP
#define _ENVELOPE_HPP

#include <vector>
#include <set>

#include "name2id.hpp"

using std::string;

#define WILDCARD (-1)

struct Envelope {
    int pid;
    int handle;
    OpType call_type;
    int count;
    int src;
    int dest;
    int stag;
    int rtag;
    std::set<int> requests;
    string comm;

    Envelope();

    Envelope(const Envelope &o);

    bool operator==(const Envelope &) const;

    bool operator!=(const Envelope &) const;

    /**
     * Defines the Intra-CB relation
     */
    bool completesBefore(Envelope const &) const;

    static Envelope ISend(int dest) {
        Envelope e;
        e.dest = dest;
        e.call_type = OpType::ISEND;
        return e;
    }

    static Envelope Barrier() {
        Envelope e;
        e.call_type = OpType::BARRIER;
        return e;
    }

    static Envelope IRecv(int src) {
        return IRecv(src, WILDCARD);
    }

    static Envelope IRecv(int src, int rtag) {
        Envelope e;
        e.src = src < 0 ? WILDCARD : src;
        e.call_type = OpType::IRECV;
        e.rtag = rtag < 0 ? WILDCARD : rtag;
        // XXX: e.count
        // XXX: e.comm
        return e;
    }

    static Envelope Wait(int req) {
        Envelope e;
        e.call_type = OpType::WAIT;
        e.requests.insert(req);
        // XXX: e.count
        return e;
    }

    bool canSend(const Envelope & recv) const;

private:
//    bool matchRecv(const Envelope & other) const;

//    bool matchSend(const Envelope & other) const;

    bool requested(int pid) const;

};

#endif
