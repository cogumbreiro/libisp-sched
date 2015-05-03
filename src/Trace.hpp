/*
 * Copyright (c) 2008-2009
 *
 * School of Computing, University of Utah,
 * Salt Lake City, UT 84112, USA
 *
 * and the Gauss Group
 * http://www.cs.utah.edu/formal_verification
 *
 * See LICENSE for licensing information
 */

#ifndef _TRACE_HPP
#define _TRACE_HPP

#include <vector>
#include <list>
#include <algorithm>
#include <sstream>
#include <memory>
#include <boost/range/adaptor/reversed.hpp>
#include <boost/optional.hpp>

#include "Envelope.hpp"
#include "Transition.hpp"

using std::unique_ptr;
using std::shared_ptr;
using std::vector;
using std::list; // XXX: remove
using boost::optional;

/*
 * Each process collects a list of transitions.
 */
class Trace {

public:
    const int pid;

    Trace(int p) : pid(p) {}

    inline unsigned int size() const { return trace.size(); }

    inline Transition & get(int index) const { return *trace[index]; }

    inline shared_ptr<Transition> getLast() const { return trace.back(); }

    bool add(unique_ptr<Envelope> t);

    inline auto begin() { return trace.begin(); }

    inline auto end() { return trace.end(); }

    inline auto begin() const { return trace.begin(); }

    inline auto end() const { return trace.end(); }

    auto reverse() {
        return boost::adaptors::reverse(trace);
    }

    vector<shared_ptr<Transition> > getRequestedProcs(const Transition &child) const {
        assert(child.pid == pid);
        vector<shared_ptr<Transition> > result;
        for (auto req_proc : child.getEnvelope().req_procs) {
            result.push_back(trace[req_proc]);
        }
        return result;
    }

    void setMatched(shared_ptr<Transition> ptr) {
        assert(ptr->pid == pid);
        ptr->setMatched();
        ulist.remove(ptr->index);
    }

    optional<shared_ptr<Transition> > findLastMatched() {
        optional<shared_ptr<Transition>> result;
        for (auto trans : reverse()) {
            if (trans->isMatched()) {
                result.reset(trans);
                return result;
            }
        }
        return result;
    }
private:
    vector<shared_ptr<Transition> > trace;

    list<int> ulist;
};

#endif
