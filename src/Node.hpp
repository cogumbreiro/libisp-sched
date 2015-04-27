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

#include "TransitionList.hpp"
#include "Matcher.hpp"

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
    Node (const Matcher & m):has_child (false), matcher(m) {}
    //Node& operator= (Node &r);
    //Node(const &Matcher matcher) { this->matcher = matcher; }

    inline int getNumProcs () const { return _num_procs; }
    inline int getLevel () const { return _level; }
    inline bool isWildcardNode() const {
        return type == WILDCARD_RECV_NODE || type == WILDCARD_PROBE_NODE;
    }
    int getTotalMpiCalls() const;
    bool allAncestorsMatched (const CB c, const std::vector<int> &l) const;
    bool anyAncestorMatched (const CB c, const std::vector<int> &l) const;
    std::vector <std::list<int> > getEnabledTransitions() const;
    bool getCollectiveAmple (const std::vector <std::list <int> > &l, int collective);
    void getWaitorTestAmple (const std::vector <std::list <int> > &l);
    bool getNonWildcardReceive (std::vector <std::list <int> > &l);
    bool getMatchingSend (CB &res, std::vector <std::list <int> > &l, CB &c);
    bool getAllMatchingSends (std::vector <std::list <int> > &l, CB &c,
                std::vector <std::list <CB> > &);
    void getallSends (std::vector <std::list <int> > &l);
    void getReceiveAmple (std::vector <std::list <int> > &l);
    bool getAmpleSet ();
//    const Transition &getTransition (int, int) const;
    void deepCopy();

    std::vector <Node *> children;
    std::vector <TransitionList*> _tlist;
    bool has_child;
    bool has_aux_coenabled_sends;
    bool tlist_dealloc;
    CB wildcard;
    NTYPE type;
    std::vector <std::list <CB> > ample_set;
    std::vector <std::list <int> > enabled_transitions;
    std::vector <std::list <CB> > other_wc_matches;
#ifdef CONFIG_BOUNDED_MIXING
    bool expand;
#endif

private:
    int _level;
    int _num_procs;
    const Matcher & matcher;
    inline const Transition & getTransition(const CB handle) const {
        return getTransition(handle.pid, handle.index);
    }
    inline const Transition & getTransition(int pid, int op_index) const {
        return _tlist[pid]->get(op_index);
    }
};

#endif
