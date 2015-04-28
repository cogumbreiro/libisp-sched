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
//    Node ();
//    Node (int num_procs);
    /*
     * Assignment Operator
     *//*
    Node (Node &);
    Node (Node &, bool copyTL);
    ~Node();
*/
    Node (bool h, const Matcher & m, NTYPE t):has_child(h), matcher(m), type(t) {}
    //Node& operator= (Node &r);
    //Node(const &Matcher matcher) { this->matcher = matcher; }

    inline int getNumProcs () const { return _num_procs; }
    inline int getLevel () const { return _level; }
    inline bool isWildcardNode() const {
        return type == WILDCARD_RECV_NODE || type == WILDCARD_PROBE_NODE;
    }
    int getTotalMpiCalls() const;
    bool addNonWildcardReceive(const vector<MPIFunc> &funcs);
    vector<list<CB> > createAllMatchingSends(const vector<MPIFunc> &funcs, MPIFunc &recv);
    void addAllSends (const vector<MPIFunc> &funcs);
    void addReceiveAmple(const vector<MPIFunc> &funcs);
    bool createAmpleSet();
    vector <list <CB> > ample_set;
    const bool has_child;
    const NTYPE type;
/*
    vector <Node *> children;
    //vector <TransitionList*> _tlist;
    bool has_aux_coenabled_sends;
    bool tlist_dealloc;
    CB wildcard;
    vector <list <int> > enabled_transitions;*/
    TransitionMap transitions;
#ifdef CONFIG_BOUNDED_MIXING
    bool expand;
#endif

private:
    int _level;
    int _num_procs;
    const Matcher & matcher;

    bool allAncestorsMatched (const MPIFunc c, const vector<int> &l) const;

    bool anyAncestorMatched (const MPIFunc c, const vector<int> &l) const;

    vector<MPIFunc> createEnabledTransitions() const;

    bool addCollectiveAmple(const vector<MPIFunc> &funcs, int collective);

    void addWaitorTestAmple(const vector<MPIFunc> &funcs);
};

#endif
