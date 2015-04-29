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
#include <set>
#include <iterator>
#include <list>
#include <cassert>
#include <boost/optional.hpp>

#include "TransitionList.hpp"
#include "Matcher.hpp"
#include "TransitionMap.hpp"
#include "EnabledTransitions.hpp"
#include "AmpleSet.hpp"

using std::vector;
using std::list;
using boost::optional;

enum NTYPE {
    GENERAL_NODE,
    WILDCARD_RECV_NODE,
    WILDCARD_PROBE_NODE,
    DEADLOCK_NODE
};

struct Node {

    Node (bool h, const Matcher & m, NTYPE t):/*has_child(h),*/ matcher(m),
    type(t),
    enabledTransitions(EnabledTransitions(m, transitions)),
    wildcard(-1,-1)
    {}
    inline int getNumProcs () const { return transitions.num_procs; }
    inline Envelope & getEnvelope(CB handle) { return transitions.getEnvelope(handle); }

    NTYPE type;
    CB wildcard;
    /*
    inline int getLevel () const { return _level; }*/
    inline bool isWildcardNode() const {
        return type == WILDCARD_RECV_NODE || type == WILDCARD_PROBE_NODE;
    }
    //int getTotalMpiCalls() const;
    vector<vector<MPIFunc> > buildAmpleSet();
    /*
    const bool has_child;
*/
/*
    vector <Node *> children;
    //vector <TransitionList*> _tlist;
    bool has_aux_coenabled_sends;
    bool tlist_dealloc;
    vector <list <int> > enabled_transitions;*/

private:
    TransitionMap transitions;
//    int _level;
//    int _num_procs;
    const Matcher & matcher;

    const EnabledTransitions enabledTransitions;
};

#endif
