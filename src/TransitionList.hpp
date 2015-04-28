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

/*
 * ISP: MPI Dynamic Verification Tool
 *
 * File:        TransitionList.hpp
 * Description: Implements lists of transitions from an interleaving
 * Contact:     isp-dev@cs.utah.edu
 */

#ifndef _TRANSITIONLIST_HPP
#define _TRANSITIONLIST_HPP

#include <vector>
#include <list>
#include <algorithm>
#include <sstream>
#include <memory>
#include <boost/iterator/indirect_iterator.hpp>

#include "Envelope.hpp"
#include "Transition.hpp"

using std::unique_ptr;
using std::vector;
using boost::indirect_iterator;
using boost::make_indirect_iterator;

/*
 * This implements a TransitionList and updates "Intra-Completes-Before"
 */
class TransitionList {

public:
    const int pid;

    TransitionList(int p) : pid(p) {}

    inline unsigned int size() const { return tlist.size(); }

    inline Transition & get(int index) const { return *tlist[index]; }

    bool addTransition (unique_ptr<Transition> t);

    inline indirect_iterator<vector<unique_ptr<Transition> >::iterator> begin () {
        return make_indirect_iterator(tlist.begin());
    }

    inline indirect_iterator<vector<unique_ptr<Transition> >::iterator> end () {
        return make_indirect_iterator(tlist.end());
    }

    inline indirect_iterator<vector<unique_ptr<Transition> >::reverse_iterator> rbegin () {
        return make_indirect_iterator(tlist.rbegin());
    }

    inline indirect_iterator<vector<unique_ptr<Transition> >::reverse_iterator> rend () {
        return make_indirect_iterator(tlist.rend());
    }

private:
    std::vector<std::unique_ptr<Transition> > tlist;

    std::list<int> ulist;

    bool intraCB (const Transition &f, const Transition &s) const;
};

#endif
