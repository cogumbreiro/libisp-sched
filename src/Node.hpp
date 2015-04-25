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
#include "TransitionList.hpp"
#include <list>
#include <cassert>
#ifndef WIN32
#include <sys/types.h>
#include <signal.h>
#endif

enum NTYPE {
    GENERAL_NODE,
    WILDCARD_RECV_NODE,
    WILDCARD_PROBE_NODE,
    DEADLOCK_NODE
};

class Node {

public:
    Node ();
    Node (int num_procs);
    /*
     * Assignment Operator
     */
    Node (Node &);
    Node (Node &, bool copyTL);
    ~Node();

    Node& operator= (Node &r);

    inline int NumProcs ();
    int GetLevel ();
    inline bool isWildcardNode() {
        return type == WILDCARD_RECV_NODE || type == WILDCARD_PROBE_NODE;
    }
    int  getTotalMpiCalls();
    bool AllAncestorsMatched (CB &c, std::vector<int> &l);
    bool AnyAncestorMatched (CB &c, std::vector<int> &l);
    void GetEnabledTransitionsOpenMP (std::vector <std::list <int> >&);
    void GetEnabledTransitionsSingleThreaded (std::vector <std::list <int> >&);
    void GetEnabledTransitionsThread (int pid, std::list<int> *l);
    bool GetCollectiveAmple (std::vector <std::list <int> > &l, int collective);
    void GetWaitorTestAmple (std::vector <std::list <int> > &l);
    bool getNonWildcardReceive (std::vector <std::list <int> > &l);

    bool getMatchingSend (CB &res, std::vector <std::list <int> > &l, CB &c);
    bool getAllMatchingSends (std::vector <std::list <int> > &l, CB &c, 
                std::vector <std::list <CB> > &);
    void GetallSends (std::vector <std::list <int> > &l);
    void GetReceiveAmple (std::vector <std::list <int> > &l);
    bool GetAmpleSet ();
    inline void setITree(ITree* new_itree);
    inline ITree* getITree();

    Transition *GetTransition (CB &c);
    Transition *GetTransition (int, int);

#ifdef USE_OPENMP
    inline void GetEnabledTransitions (std::vector <std::list <int> > &l) {
        if (Scheduler::_openmp) {
            GetEnabledTransitionsOpenMP(l);
        } else {
            GetEnabledTransitionsSingleThreaded(l);
        }
    }
#else
#define GetEnabledTransitions GetEnabledTransitionsSingleThreaded
#endif

    friend std::ostream &operator<< (std::ostream &os, Node &n);
    void deepCopy();

    std::vector <Node *> children;
//    Node *next_sibling;
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
    ITree* itree;
};

#endif
