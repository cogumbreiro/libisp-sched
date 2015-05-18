#ifndef _CALL_DB_HPP
#define _CALL_DB_HPP

#include <set>
#include <boost/optional/optional.hpp>

#include "Call.hpp"

using std::set;
using std::vector;
using boost::optional;

struct Schedule {
    /**
     * The calls to be scheduled;
     */
    vector<Call> calls;
    /**
     * The number of processes in the system.
     */
    int procs;
    /**
     * For each communicator set the number of participants
     */
    map<int, int> participants;
};

struct CallDB {
    CallDB(const Schedule & schedule);

    void add(const Call &call);

    vector<Call> findCollective() const;

    vector<Call> getCollective(CallType type, int comm);

    vector<Call> findWait() const;

    vector<Call> findReceive() const;

    vector<Call> findReceiveAny() const;

    vector<Call> findFinalize() const;

    vector<Call> getFinalize() const;

    // get a send if it matches the given receive
    optional<Call> matchReceive(const Call &) const;
    // get all send that match this receive any
    vector<Call> matchReceiveAny(const Call &) const;

    int participantsFor(int) const;

private:
    int procs;
    map<int, int> participants;
    map<CallType, map<int, vector<Call> > > collective;
    vector<Call> finalize;
    vector<Call> receive;
    vector<Call> receiveAny;
    vector<Call> send;
    vector<Call> wait;
    void addFinalize(const Call &);
    void addCollective(const Call &);
    void addReceive(const Call&);
    void addReceiveAny(const Call&);
    void addWait(const Call&);
    void addSend(const Call&);
};

#endif
