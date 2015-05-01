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
#include <algorithm>
#include <boost/range/adaptor/reversed.hpp>
#include <boost/range/adaptor/indirected.hpp>

#include "Transition.hpp"
#include "Trace.hpp"

using std::vector;
using std::list;
using std::unique_ptr;
using std::move;
using std::make_unique;
using boost::adaptors::reverse;
using boost::adaptors::indirect;
using boost::make_indirect_iterator;

struct State {
public:
    const int num_procs;

    State() : num_procs(0) {}

    State(int p) : num_procs(p) {
        for (int pid = 0; pid < num_procs; pid++) {
            traces.push_back(move(make_unique<Trace>(pid)));
        }
    }

    inline auto begin() {
        return make_indirect_iterator(traces.begin());
    }

    inline auto end() {
        return make_indirect_iterator(traces.end());
    }

    inline auto begin() const {
        return make_indirect_iterator(traces.begin());
    }

    inline auto end() const {
        return make_indirect_iterator(traces.end());
    }

    inline shared_ptr<Transition> getLast(int pid) const {
        return traces[pid]->getLast();
    }

    void setMatched(shared_ptr<Transition> ptr) {
        traces[ptr->pid]->setMatched(ptr);
    }

    vector<shared_ptr<Transition> > getRequestedProcs(const Transition &child) const {
        return traces[child.pid]->getRequestedProcs(child);
    }

    unsigned int getMaxSize() {
        unsigned int size = 0;
        for (Trace & trace : indirect(traces)) {
            size = std::max(trace.size(), size);
        }
        return size;
    }

private:
    vector<unique_ptr<Trace>> traces;
};

#endif
