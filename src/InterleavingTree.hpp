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
#ifndef _INTERLEAVING_TREE_HPP
#define _INTERLEAVING_TREE_HPP

#define CONSOLE_OUTPUT_THRESHOLD 200

#include <vector>
#include <set>
#include <iterator>
#include <list>
#include <cassert>

#include "TransitionList.hpp"

class Node;

class ITree {
public:
    ITree (Node *, std::string);
    int CHECK (std::list <int> &);
    Node *GetCurrNode ();
    bool NextInterleaving ();
    void resetDepth ();
    void ResetMatchingInfo();
    void printTypeMismatches();//CGD
    std::list<CB> mismatchTypeList;//CGD
    int GetCurrentDepth() { return depth; }

    void ProcessInterleaving();

    std::vector <bool* > is_matched;
    std::vector <bool* > is_issued;
    int* last_matched;
    std::map <CB, std::list <CB> > aux_coenabled_sends;
    std::map <CB, std::set <CB> > matched_sends;
    bool fprs = true;
#ifdef CONFIG_BOUNDED_MIXING
    unsigned expanded;
#endif
  bool                            *_is_exall_mode;

private:
    int depth;
    bool have_wildcard;
    std::string pname;
    std::vector <Node *> _slist;
    static const int MAX_TRANSITIONS = 500000;
    void AddInterCB();
    void ClearInterCB();
    void FindCoEnabledSends();
    bool FindNonSendWaitPath(bool **visited, CB &src, CB &dest);
    bool findSendOfThisWait(CB& res, CB& c);

    size_t getMaxTlistSize();
};
#endif
