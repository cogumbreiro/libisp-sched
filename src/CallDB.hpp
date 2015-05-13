#ifndef _CALL_DB_HPP
#define _CALL_DB_HPP

#include <set>
#include <boost/optional/optional.hpp>

#include "Call.hpp"

using std::set;
using std::vector;
using boost::optional;

struct CallDB {
    CallDB(const set<Call> & enabled);

    void add(const Call &call);

    vector<Call> findCollective() const;

    vector<Call> findWait() const;

    vector<Call> findReceive() const;

    vector<Call> findReceiveAny() const;

    // get a send if it matches the given receive
    optional<Call> matchReceive(const Call &) const;
    // get all send that match this receive any
    vector<Call> matchReceiveAny(const Call &) const;

private:
    vector<Call> collective;
    vector<Call> receive;
    vector<Call> receiveAny;
    vector<Call> send;
    vector<Call> wait;
    void addCollective(const Call &);
    void addReceive(const Call&);
    void addReceiveAny(const Call&);
    void addWait(const Call&);
    void addSend(const Call&);
};

#endif
