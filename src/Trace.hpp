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

#include "Envelope.hpp"
#include "Transition.hpp"

using std::unique_ptr;
using std::shared_ptr;
using std::vector;
using std::list; // XXX: remove

/*
 * Each process collects a list of transitions.
 */
class Trace {

public:
    const int pid;

    Trace(int p) : pid(p) {}

    inline unsigned int size() const { return tlist.size(); }

    inline Transition & get(int index) const { return *tlist[index]; }

    inline Transition & getLast() const { return *tlist.back(); }

    bool add(unique_ptr<Envelope> t);

    inline auto begin() { return tlist.begin(); }

    inline auto end() { return tlist.end(); }

    inline auto begin() const { return tlist.begin(); }

    inline auto end() const { return tlist.end(); }

    auto reverse() {
        return boost::adaptors::reverse(tlist);
    }

    vector<shared_ptr<Transition> > getRequestedProcs(const Transition &child) const {
        assert(child.pid == pid);
        vector<shared_ptr<Transition> > result;
        for (auto req_proc : child.getEnvelope().req_procs) {
            result.push_back(tlist[req_proc]);
        }
        return result;
    }


private:
    vector<shared_ptr<Transition> > tlist;

    list<int> ulist;

    bool intraCB (const Transition &f, const Transition &s) const;
};

#endif