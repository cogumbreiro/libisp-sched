#ifndef _CALL_HPP
#define _CALL_HPP

#include <vector>
#include <map>
#include <set>
#include <utility>

#include "name2id.hpp"
#include "Envelope.hpp"

using std::set;
using std::vector;
using std::map;

struct Recv {
    int count;
    int datatype;
    int source;
    int tag;
    int comm;
};

struct Send {
    int count;
    int datatype;
    int dest;
    int tag;
    int comm;
};

struct Sendrecv {
    int sendcount;
    int sendtype;
    int dest;
    int sendtag;
    int recvcount;
    int recvtype;
    int source;
    int recvtag;
    int comm;
};

struct Wait {
    std::set<int> request;
};

struct Bcast {
    int count;
    int datatype;
    int root;
    int comm;
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
    /** the logical time at which this call has been issued (monotonic). */
    int handle;
    /** The payload of the MPI call */
    Envelope envelope;
    /** Defines each field of this object. */
    Call(int p, int i, const Envelope &env);
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
#endif
