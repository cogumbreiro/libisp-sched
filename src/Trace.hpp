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
#include <boost/iterator/indirect_iterator.hpp>

#include "Envelope.hpp"
#include "Transition.hpp"

using std::unique_ptr;
using std::shared_ptr;
using std::vector;
using std::list; // XXX: remove
using boost::indirect_iterator;
using boost::make_indirect_iterator;

/*
 * Each process collects a list of transitions.
 */
class Trace {

public:
    const int pid;

    Trace(int p) : pid(p) {}

    inline unsigned int size() const { return tlist.size(); }

    inline Transition & get(int index) const { return *tlist[index]; }

    bool add(unique_ptr<Envelope> t);

    inline indirect_iterator<vector<shared_ptr<Transition> >::iterator> begin () {
        return make_indirect_iterator(tlist.begin());
    }

    inline indirect_iterator<vector<shared_ptr<Transition> >::iterator> end () {
        return make_indirect_iterator(tlist.end());
    }

    inline indirect_iterator<vector<shared_ptr<Transition> >::reverse_iterator> rbegin () {
        return make_indirect_iterator(tlist.rbegin());
    }

    inline indirect_iterator<vector<shared_ptr<Transition> >::reverse_iterator> rend () {
        return make_indirect_iterator(tlist.rend());
    }

private:
    vector<shared_ptr<Transition> > tlist;

    list<int> ulist;

    bool intraCB (const Transition &f, const Transition &s) const;
};

#endif
