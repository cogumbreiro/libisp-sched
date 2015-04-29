/*
 * Tiago Cogumbreiro (cogumbreiro@users.sf.net) Copyright (c) 2015
 *
 * See LICENSE for licensing information
 */

#ifndef _TRANSITION_MAP_HPP
#define _TRANSITION_MAP_HPP

#include <vector>
#include <memory>
#include <utility>
#include <boost/range/adaptor/reversed.hpp>
#include <boost/range/adaptor/indirected.hpp>

#include "CB.hpp"
#include "Transition.hpp"
#include "TransitionList.hpp"

using std::vector;
using std::list;
using std::unique_ptr;
using std::move;
using std::make_unique;
using boost::adaptors::reverse;
using boost::adaptors::indirect;

struct MPIFunc {
    MPIFunc(const MPIFunc &o) : handle(o.handle), envelope(o.envelope){}
    MPIFunc(CB h, Envelope & e) : handle(h), envelope(e) {}
    CB handle;
    Envelope & envelope;
    MPIFunc& operator=(const MPIFunc& other) {
        handle = other.handle;
        envelope = other.envelope;
        return *this;
    }
    inline bool canSend(MPIFunc & recv) const {
        return handle.pid == recv.envelope.src &&
                envelope.canSend(recv.envelope);
    }
};

struct TransitionMap {

    const int num_procs;

    TransitionMap() : num_procs(0) {}

    TransitionMap(int p) : num_procs(p) {
        for (int pid = 0; pid < num_procs; pid++) {
            transitions.push_back(move(make_unique<TransitionList>(pid)));
        }
    }

    inline Envelope & getEnvelope(const CB handle) const {
        return transitions[handle.pid]->get(handle.index).getEnvelope();
    }

    inline Transition & get(const CB handle) {
        return transitions[handle.pid]->get(handle.index);
    }

    inline const Transition & get(const CB handle) const {
        return transitions[handle.pid]->get(handle.index);
    }

    inline MPIFunc getLast(int pid) const {
        auto & trans = transitions[pid];
        return getMPIFunc(CB(pid, trans->size() - 1));
    }

    vector<vector<MPIFunc> > generateMPIFuncs() const {
        vector<vector<MPIFunc> > result;
        for (int pid = 0 ; pid < num_procs; pid++) {
            vector<MPIFunc> proc;
            populateProc(pid, proc);
            result.push_back(proc);
        }
        return result;
    }

private:
    vector <unique_ptr<TransitionList>> transitions;

    inline MPIFunc getMPIFunc(const CB handle) const {
        return MPIFunc(handle, getEnvelope(handle));
    }

    inline void populateProc(int pid, vector<MPIFunc> &result) const {
        int idx = 0;
        for (const Transition & trans : *transitions[pid]) {
            CB handle(pid, idx);
            result.push_back(getMPIFunc(handle));
            idx++;
        }
    }
};

#endif
