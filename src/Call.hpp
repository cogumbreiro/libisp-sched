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

using std::set;
using std::vector;
using std::map;

struct WInt {
    WInt() : wildcard(true), value(0)  {}
    WInt(int v) : wildcard(false), value(v) {}
    WInt(const WInt &c) : wildcard(c.wildcard), value(c.value) {}
    bool operator== (const WInt &r) const {
        return wildcard == r.wildcard && value == r.value;
    }

    WInt & operator=(const WInt & other) {
        wildcard = other.wildcard;
        value = other.value;
        return *this;
    }

    bool isWildcard() const {
        return wildcard;
    }
    int get() const {
        return value;
    }
    bool matches(const WInt &other) const {
        return isWildcard() || *this == other;
    }
private:
    bool wildcard;
    int value;
};

static WInt WILDCARD = WInt();

struct Recv {
    Recv() : src(0), tag(0) {}
    Recv(const Recv &r) : src(r.src), tag(r.tag) {}

    WInt src;
    WInt tag;
    inline bool completesBefore(const Recv &rhs) const {
        return src.matches(rhs.src) && tag.matches(rhs.tag);
    }
    bool operator== (const Recv &r) const {
        return src == r.src && tag == r.tag;
    }
};

struct Send {
    Send() : dest(0), tag(0) {}
    Send(const Send &s) : dest(s.dest), tag(s.tag) {}

    int dest;
    int tag;
    inline bool completesBefore(const Send &rhs) const {
        return dest == rhs.dest && rhs.tag == tag;
    }

    bool canSend(const Recv & recv) const {
        return recv.src.matches(dest) && recv.tag.matches(tag);
    }

    bool operator== (const Send &s) const {
        return dest == s.dest && tag == s.tag;
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

enum class Field {
    Source,
    Destination,
    Datatype,
    Root,
    Communicator,
    Op,
    Count,
    Sendcount,
    Recvcount,
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
    map<Field, int> metadata;
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
        return IRecv(WInt(src));
    }

    Call IRecv(WInt src) {
        return IRecv(0, 0, src, WILDCARD, 0);
    }

    Call IRecv(int count, int datatype, WInt src, WInt rtag, int comm) {
        Call c = create();
        c.recv.src = src;
        c.call_type = OpType::IRECV;
        c.recv.tag = rtag;
        c.metadata[Field::Count] = count;
        c.metadata[Field::Datatype] = datatype;
        c.metadata[Field::Communicator] = comm;
        return c;
    }

    Call Recv(int count, int datatype, WInt src, WInt rtag, int comm) {
        Call c = create();
        c.recv.src = src;
        c.call_type = OpType::RECV;
        c.recv.tag = rtag;
        c.metadata[Field::Count] = count;
        c.metadata[Field::Datatype] = datatype;
        c.metadata[Field::Communicator] = comm;
        return c;
    }

    Call Send(int count, int datatype, int dest, int tag, int comm) {
        Call c = create();
        c.call_type = OpType::SEND;
        c.send.dest = dest;
        c.send.tag = tag;
        c.metadata[Field::Count] = count;
        c.metadata[Field::Datatype] = datatype;
        c.metadata[Field::Communicator] = comm;
        return c;
    }

    Call Ssend(int count, int datatype, int dest, int tag, int comm) {
        Call c = create();
        c.call_type = OpType::SSEND;
        c.send.dest = dest;
        c.send.tag = tag;
        c.metadata[Field::Count] = count;
        c.metadata[Field::Datatype] = datatype;
        c.metadata[Field::Communicator] = comm;
        return c;
    }

    Call Isend(int count, int datatype, int dest, int tag, int comm) {
        Call c = create();
        c.call_type = OpType::ISEND;
        c.send.dest = dest;
        c.send.tag = tag;
        c.metadata[Field::Count] = count;
        c.metadata[Field::Datatype] = datatype;
        c.metadata[Field::Communicator] = comm;
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
