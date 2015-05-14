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

struct Receive {
    Receive() : count(0), datatype(0), src(0), tag(0), comm(0) {}
    Receive(const Receive &r) : count(r.count), datatype(r.datatype), src(r.src),
        tag(r.tag), comm(r.comm) {}
    Receive(int count, int datatype, WInt src, WInt tag, int comm) :
        count(count), datatype(datatype), src(src), tag(tag), comm(comm) {}

    int count;
    int datatype;
    WInt src;
    WInt tag;
    int comm;

    inline bool matches(const Receive &rhs) const {
        return count == rhs.count
            && datatype == rhs.datatype
            && src.matches(rhs.src)
            && tag.matches(rhs.tag)
            && comm == rhs.comm;


    }
    bool operator== (const Receive &r) const {
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
    Send(int count, int datatype, int dest, int tag, int comm) : count(count), datatype(datatype), dest(dest),
            tag(tag), comm(comm) {}

    int count;
    int datatype;
    int dest;
    int tag;
    int comm;

    bool canSend(const Receive & recv) const {
        return count == recv.count
                && datatype == recv.datatype
                && recv.src.matches(dest)
                && recv.tag.matches(tag)
                && recv.comm == comm;
    }

    bool operator== (const Send &s) const {
        return count == s.count
                && datatype == s.datatype
                && dest == s.dest
                && tag == s.tag
                && comm == s.comm;
    }

};

struct Collective {
    Collective() {}
    Collective(const Collective &c) : data(c.data) {}

    void set(Field f, int v) {
        data[f] = v;
    }

    optional<int> get(Field f) const {
        optional<int> result;
        auto iter = data.find(f);
        if (iter != data.end()) {
            result.reset(iter->second);
        }
        return result;
    }

    bool operator==(const Collective &c) {
        return data == c.data;
    }

private:
    map<Field, int> data;
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
    Receive recv;
    Send send;
    Wait wait;
    Collective collective;
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

/**
 * @brief A factory of Call objects, should be used per process.
 */
struct Process {
    int pid;
    int curr_handle;
    Process(int pid) : pid(pid), curr_handle(0) {}

    Call irecv(int count, int datatype, WInt src, WInt tag, int comm) {
        return create(OpType::IRECV, Receive(count, datatype, src, tag, comm));
    }

    Call irecv(int src) {
        return irecv(WInt(src));
    }

    Call irecv(WInt src) {
        return irecv(0, 0, src, WILDCARD, 0);
    }

    Call recv(int count, int datatype, WInt src, WInt tag, int comm) {
        return create(OpType::RECV, Receive(count, datatype, src, tag, comm));
    }

    Call send(int count, int datatype, int dest, int tag, int comm) {
        return create(OpType::SEND, Send(count, datatype, dest, tag, comm));
    }

    Call ssend(int count, int datatype, int dest, int tag, int comm) {
        return create(OpType::SSEND, Send(count, datatype, dest, tag, comm));
    }

    Call isend(int count, int datatype, int dest, int tag, int comm) {
        return create(OpType::ISEND, Send(count, datatype, dest, tag, comm));
    }

    Call isend(int dest) {
        return isend(0, 0, dest, 0, 0);
    }

    Call rsend(int count, int datatype, int dest, int tag, int comm) {
        return create(OpType::RSEND, Send(count, datatype, dest, tag, comm));
    }

    Call wait(int request) {
        Call c = create(OpType::WAIT);
        c.wait.addRequest(request);
        return c;
    }

    Call barrier(int comm) {
        Call c = create(OpType::BARRIER);
        c.collective.set(Field::Communicator, comm);
        return c;
    }

    Call barrier() {
        return barrier(0);
    }

    Call finalize() {
        return create(OpType::FINALIZE);
    }


private:

    Call create(OpType call_type) {
        Call c;
        c.pid = pid;
        c.handle = curr_handle++;
        c.call_type = call_type;
        return c;
    }

    Call create(OpType call_type, const Receive &recv) {
        Call c = create(call_type);
        c.recv = recv;
        return c;
    }

    Call create(OpType call_type, const Send &send) {
        Call c = create(call_type);
        c.send = send;
        return c;
    }
};
#endif
