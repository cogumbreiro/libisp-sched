#ifndef _CALL_DB_HPP
#define _CALL_DB_HPP

#include <set>
#include <boost/optional/optional.hpp>

#include "Envelope.hpp"
#include "Call.hpp"

using std::set;
using std::vector;
using boost::optional;

/* This enumerator is used internally to categorize the calls. */
enum class MPIKind {
    Collective,
    ReceiveAny,
    Receive,
    Send,
    Wait,
    Unknown
};

/* Given an evelope return the associated match. */
MPIKind to_kind(const Envelope &env);

struct CallDB {
    CallDB(const set<Call> & enabled);

    void add(Call &call);

    vector<Call> findCollective() const;

    vector<Call> findWait() const;

    vector<Call> findReceive() const;

    vector<Call> findReceiveAny() const;

    // get a send if it matches the given receive
    optional<Call> matchReceive(const Envelope &) const;
    // get all send that match this receive any
    vector<Call> matchReceiveAny(const Envelope &) const;

private:
    vector<Call> at(const MPIKind key) const;

    map<MPIKind, vector<Call> > data;
};

#endif
