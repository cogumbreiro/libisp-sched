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
 * File:        InterleavingTree.hpp
 * Description: Implements ISP's POE algorithm
 * Contact:     isp-dev@cs.utah.edu
 */

#ifndef _NODE_HPP
#define _NODE_HPP

#define CONSOLE_OUTPUT_THRESHOLD 200

#include <vector>
#include <memory>
#include <set>
#include <iterator>
#include <list>
#include <cassert>
#include <boost/optional.hpp>

#include "Trace.hpp"
#include "State.hpp"
#include "EnabledTransitions.hpp"
#include "AmpleSet.hpp"

using std::vector;
using std::list;
using std::shared_ptr;
using std::unique_ptr;
using boost::optional;

enum NTYPE {
    GENERAL_NODE,
    WILDCARD_RECV_NODE,
    WILDCARD_PROBE_NODE,
    DEADLOCK_NODE
};

struct Node {

    Node(unique_ptr<State> s, bool h, NTYPE t):
    state(std::move(s)),
    type(t),
    enabledTransitions(EnabledTransitions(*s))//,
    {}
    inline int getNumProcs () const { return state->num_procs; }

    NTYPE type;
    shared_ptr<Transition> wildcard;
    /*
    inline int getLevel () const { return _level; }*/
    inline bool isWildcardNode() const {
        return type == WILDCARD_RECV_NODE || type == WILDCARD_PROBE_NODE;
    }
    //int getTotalMpiCalls() const;
    vector<vector<shared_ptr<Transition> > > buildAmpleSet();
    /*
    const bool has_child;
*/
/*
    vector <Node *> children;
    //vector <TransitionList*> _tlist;
    bool has_aux_coenabled_sends;
    bool tlist_dealloc;
    vector <list <int> > enabled_transitions;*/
    State & getState() { return *state; };

private:
    unique_ptr<State> state;
//    int _level;
//    int _num_procs;

    const EnabledTransitions enabledTransitions;
};

#endif
