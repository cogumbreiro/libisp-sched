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

class Node {

public:
    Node (bool h, const Matcher & m, NTYPE t):/*has_child(h),*/ matcher(m), /*type(t),*/
    enabledTransitions(EnabledTransitions(matcher, transitions))
    {}
    /*
    inline int getNumProcs () const { return _num_procs; }
    inline int getLevel () const { return _level; }*/
    /*
    inline bool isWildcardNode() const {
        return type == WILDCARD_RECV_NODE || type == WILDCARD_PROBE_NODE;
    }*/
    //int getTotalMpiCalls() const;
    vector<vector<CB> > createAmpleSet();
    vector <list <CB> > ample_set;/*
    const bool has_child;
    const NTYPE type;
*/
/*
    vector <Node *> children;
    //vector <TransitionList*> _tlist;
    bool has_aux_coenabled_sends;
    bool tlist_dealloc;
    CB wildcard;
    vector <list <int> > enabled_transitions;*/

private:
    TransitionMap transitions;
//    int _level;
//    int _num_procs;
    const Matcher & matcher;

    const EnabledTransitions & enabledTransitions;
};

#endif
