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
 * See LICENSE.ISP for licensing information
 */

#ifndef _CALL_HPP
#define _CALL_HPP

#include <vector>
#include <map>
#include <set>
#include <utility>

#include "name2id.hpp"

#define WILDCARD (-1)

using std::set;
using std::vector;
using std::map;

struct Recv {
    Recv() : count(0), datatype(0), src(0), tag(0), comm(0) {}
    Recv(const Recv &r) : count(r.count), datatype(r.datatype),
        src(r.src), tag(r.tag), comm(r.comm) {}

    int count;
    int datatype;
    int src;
    int tag;
    int comm;
    inline bool completesBefore(const Recv &rhs) const {
        return (src == rhs.src || src == WILDCARD)
                && comm == rhs.comm
                && (tag == rhs.tag || tag == WILDCARD);
    }
    bool operator== (const Recv &r) const {
        return count == r.count
                && datatype == r.datatype
                && src == r.src
                && tag == r.tag
                && comm == r.comm;
    }
};

struct Send {
    Send() : count(0), datatype(0), dest(0), tag(0), comm(0) {}
    Send(const Send &s) : count(s.count), datatype(s.datatype), dest(s.dest),
        tag(s.tag), comm(s.comm) {}

    int count;
    int datatype;
    int dest;
    int tag;
    int comm;
    inline bool completesBefore(const Send &rhs) const {
        return dest == rhs.dest
            && (tag == WILDCARD || tag == rhs.tag)
            && comm == rhs.comm;
    }

    bool canSend(const Recv & recv) const {
        return comm == recv.comm &&
            count == recv.count &&
            (dest == recv.src || recv.src == WILDCARD) &&
            (tag == recv.tag || recv.tag == WILDCARD);
    }

    bool operator== (const Send &s) const {
        return count == s.count
                && datatype == s.datatype
                && dest == s.dest
                && tag == s.tag
                && comm == s.comm;
    }

};

struct Wait {
    Wait(): requests() {}
    Wait(const Wait &w) : requests(w.requests) {}

    inline bool requested(int handle) const {
        return requests.find(handle) != requests.end();
    }
    void addRequest(int handle) {
        requests.insert(handle);
    }
    bool operator== (const Wait &w) const {
        return requests == w.requests;
    }
private:
    std::set<int> requests;
};

/*
 * Represents an MPI call issued by a certain process with a `pid`.
 * Each process has a logical closs to uniquely identify its issuing calls,
 * here it is defined as field `cid`.
 * The `envelope` holds information about the MPI call.
 */
struct Call {
    /** The process id of the issuer. */
    int pid;
    /** The logical time at which this call has been issued (monotonic). */
    int handle;
    /** The op type defines which payload to use */
    OpType call_type;
    /** The payload of the MPI call */
    Recv recv;
    Send send;
    Wait wait;
    /** Defines each field of this object. */
    Call(int p, int i);
    /** Copy ctor as one would expect. */
    Call(const Call & c);
    /** Default ctor. */
    Call();
    /** Checks if this call precedes another with the completes-before rel */
    bool completesBefore(Call const&) const;
    /** Checks if pid and cid are the same. */
    bool operator== (const Call &c) const;
    /** Default. */
    bool operator!= (const Call &c) const;
    /** Checks if this call can send to the given receive */
    bool canSend(const Call & recv) const;
    /**
     * pre-condition: all members of `call` must have the same `pid` as this
     * instance.
     */
    bool hasAncestors(const set<Call> & call) const;
    /** Holds if the pids are smaller or, when they match if the cid is smaller. */
    friend bool operator< (const Call &a, const Call &b) {
        return a.pid < b.pid || (a.pid == b.pid && a.handle < b.handle);
    }
};

struct Process {
    int pid;
    int curr_handle;
    Process(int pid) : pid(pid), curr_handle(0) {}

    Call create() {
        return Call(pid, curr_handle++);
    }

    // REMOVE THIS FROM HERE:

    Call ISend(int dest) {
        Call c = create();
        c.send.dest = dest;
        c.call_type = OpType::ISEND;
        return c;
    }

    Call Barrier() {
        Call c = create();
        c.call_type = OpType::BARRIER;
        return c;
    }

    Call IRecv(int src) {
        return IRecv(src, WILDCARD);
    }

    Call IRecv(int src, int rtag) {
        Call c = create();
        c.recv.src = src < 0 ? WILDCARD : src;
        c.call_type = OpType::IRECV;
        c.recv.tag = rtag < 0 ? WILDCARD : rtag;
        // XXX: e.count
        // XXX: e.comm
        return c;
    }

    Call Wait(int req) {
        Call c = create();
        c.call_type = OpType::WAIT;
        c.wait.addRequest(req);
        // XXX: e.count
        return c;
    }
};
#endif
