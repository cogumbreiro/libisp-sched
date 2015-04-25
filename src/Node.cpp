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

#include <sstream>
#include <map>
#include <queue>
#include <string.h>

#include "InterleavingTree.hpp"

Node::Node ():has_child (false) {
}

Node::Node (int num_procs) : has_child (false), _level(0),
        _num_procs(num_procs) {

    type = GENERAL_NODE;
#ifdef CONFIG_BOUNDED_MIXING
    expand = false;
#endif
    has_aux_coenabled_sends = false;
    for (int i = 0; i< num_procs ;i++) {
        _tlist.push_back(new TransitionList (i));
    }
}

Node::Node (Node &n) {
    *this = n;
}

Node::Node (Node &n, bool copyTL) {
    if (copyTL) {
        tlist_dealloc = true;
        for (int i = 0 ; i < _num_procs; i++) {
            TransitionList *tl = new TransitionList (*(n._tlist[i]));
            _tlist.push_back (tl);
        }
    } else {
        for (int i = 0 ; i < _num_procs; i++) {
            _tlist.push_back (n._tlist[i]);
        }
    }

    _num_procs = n.NumProcs();
    type = GENERAL_NODE;
    _level = (n.GetLevel())+1;
    itree = n.itree;
    has_child = false;
    *this = n;  
}

Node::~Node() {
    int i;
        for(i=0; i<_num_procs; i++)
            delete this->_tlist[i];
}


Node &Node::operator= (Node &n) {
    _num_procs = n.NumProcs ();
    for (int i = 0 ; i < _num_procs; i++) {
        _tlist.push_back(new TransitionList(*n._tlist[i]));
    }
    _level = (n.GetLevel())+1;
    has_child = false;
    itree = n.itree;
#ifdef CONFIG_BOUNDED_MIXING
    expand = false;
#endif
    return *this;
}

void Node::deepCopy() {
    int i;
    std::vector <TransitionList*> tlist;
    for(i=0; i<_num_procs; i++) {
        TransitionList *tl = new TransitionList(*(this->_tlist[i]));
        tlist.push_back(tl);
    }
    _tlist = tlist;
    tlist_dealloc = true;
}

void Node::setITree(ITree* new_itree) {
    itree = new_itree;
}

ITree* Node::getITree() {
    return itree;
}

int Node::NumProcs () {
    return _num_procs;
}

int Node::GetLevel () {
    return _level;
}

int Node::getTotalMpiCalls() {
    int sum = 0;
    std::vector <TransitionList *> ::iterator iter;
    std::vector <TransitionList *> ::iterator iter_end = _tlist.end();

    for (iter=_tlist.begin(); iter != iter_end; iter++) {
        sum += (*iter)->_tlist.size();
    }
    return sum;
}

bool Node::AllAncestorsMatched (CB &c, std::vector <int> &l) {
   
    bool is_wait_or_test_type = GetTransition(c)->GetEnvelope()->isWaitorTestType();

    std::vector <int>::iterator iter, iter2, iter2_end;
    std::vector <int>::iterator iter_end = l.end();
    for (iter = l.begin (); iter != iter_end; iter++) {
        if (!itree->is_matched[c._pid][(*iter)]) {
// Wei-Fan Chiang: I don't know why I need to add this line. But, I need it........
            if (_tlist[c._pid]->_tlist[(*iter)].GetEnvelope ()->func_id == PCONTROL) continue;
            if (is_wait_or_test_type && !Scheduler::_send_block) {
                if (_tlist[c._pid]->_tlist[(*iter)].GetEnvelope ()->func_id == ISEND) {
                        continue;
                }
            }
            return false;
        } else if (_tlist[c._pid]->_tlist[*iter].GetEnvelope()->func_id == WAIT ||
                   _tlist[c._pid]->_tlist[*iter].GetEnvelope()->func_id == TEST ||
                   _tlist[c._pid]->_tlist[*iter].GetEnvelope()->func_id == WAITALL ||
                   _tlist[c._pid]->_tlist[*iter].GetEnvelope()->func_id == TESTALL) {

            Transition t = _tlist[c._pid]->_tlist[*iter];
            iter2_end = t.get_ancestors().end();
            for (iter2 = t.get_ancestors().begin (); iter2 != iter2_end; iter2++) {
                Envelope* env_f = _tlist[c._pid]->_tlist[*iter2].GetEnvelope();
                Envelope* env_s = GetTransition (c)->GetEnvelope();
                if(!itree->is_matched[c._pid][(*iter2)] &&
                    env_f->isSendType () && env_s->isSendType () &&
                    env_f->dest == env_s->dest && env_f->comm == env_s->comm &&
                    env_f->stag == env_s->stag) {
                    return false;
                }
            }
        }
    }
    return true;
}

bool Node::AnyAncestorMatched (CB &c, std::vector<int> &l) {
    std::vector <int>::iterator it;

    bool any_match = false;

    Envelope *e = GetTransition (c)->GetEnvelope ();

    std::vector <int>::iterator iter;
    std::vector <int>::iterator iter_end = l.end();

    std::vector <int>::iterator it_end = e->req_procs.end();

    bool is_wait_or_test_type = GetTransition(c)->GetEnvelope()->isWaitorTestType();
    if (is_wait_or_test_type && l.size () == 0) {
        return true;
    }

    for (it = e->req_procs.begin (); it != it_end; it++) {
        if (itree->is_matched[c._pid][*it] || 
            (is_wait_or_test_type && !Scheduler::_send_block && _tlist[c._pid]->_tlist[*it].GetEnvelope()->func_id == ISEND)) {
            any_match = true;
        }
        for (iter = l.begin (); iter != iter_end; iter++) {
            if ((*iter) == *it) {
                iter = l.erase (iter);
                iter_end = l.end ();
                break;
            }
        }
    }
    if (!any_match) {
        return false;
    }

    iter_end = l.end();

    for (iter = l.begin (); iter != iter_end; iter++) {
        if(!itree->is_matched[c._pid][*iter]) {
            return false;
        }
    }

    return true;
}


/* This is the exact same as the above, just with OpenMP disabled. */
/* Note that the use of reverse iterator here is for performance reason 
 * so that we can break off the loop when we hit the last_matched
 * transition - If we stop using reverse iterator - we need to stop using
 * reverse iterator in GetMatchingSends as well - otherwise we won't 
 * be able to preserve the program order matching of receives/sends */
void Node::GetEnabledTransitionsSingleThreaded (std::vector <std::list <int> > &l) {
    l.clear ();

    for (int i = 0; i < NumProcs(); i++) {
        l.push_back (std::list <int> ());
    }

    for (int i = 0 ; i < NumProcs () ;i++) {
        std::vector <Transition>::reverse_iterator iter =
            _tlist[i]->_tlist.rbegin();
        std::vector <Transition>::reverse_iterator iter_end;
        CB c(i,0);

        iter_end = _tlist[i]->_tlist.rend();
        int j = (int)_tlist[i]->_tlist.size()-1;
        for (; iter != iter_end; iter++) {
            c._index = j;
            if (!itree->is_matched[i][j]) {
                std::vector<int> &ancestor_list(GetTransition(c)->get_ancestors());
                if ((iter->GetEnvelope ()->func_id != WAITANY &&
                        iter->GetEnvelope ()->func_id != TESTANY) &&
                        AllAncestorsMatched (c,ancestor_list)) {
                    l[i].push_back (j);
                } else if ((iter->GetEnvelope()->func_id == WAITANY ||
                        iter->GetEnvelope()->func_id == TESTANY) &&
                        AnyAncestorMatched (c,ancestor_list)) {
                    l[i].push_back (j);

                }
            }
            j--;
            if (j <= itree->last_matched[i])
                break;
            
        }
    }

}

void Node::GetWaitorTestAmple (std::vector <std::list <int> > &l) {
    std::list <CB> blist;
    for (int i = 0; i < NumProcs (); i++) {
        std::list <int>::iterator iter;
        std::list <int>::iterator iter_end;
        Envelope *e;
        iter_end = l[i].end();
        for (iter = l[i].begin (); iter != iter_end; iter++) {
            e = GetTransition(i, (*iter))->GetEnvelope();

            if (e->isWaitorTestType ()) {
                blist.push_back (CB(i, *iter));
            }
        }
    }
    if (!blist.empty ()) {
        ample_set.push_back (blist);

    }
}

bool Node::GetCollectiveAmple (std::vector <std::list <int> > &l, int collective) {
    std::list <CB> blist;
    std::list <CB> flist;

    for (int i = 0; i < NumProcs (); i++) {
        std::list <int>::iterator iter;
        std::list <int>::iterator iter_end;
        
        iter_end = l[i].end();
        for (iter = l[i].begin (); iter != iter_end; iter++) {
            if (GetTransition (i, *iter)->GetEnvelope ()->func_id == collective) {
                blist.push_back (CB(i, *iter));
            }
        }
    }

    if (collective == FINALIZE) {
        if ((int)blist.size () == NumProcs ()) {
            ample_set.push_back (blist);
            return true;
        }
        return false;
    }

    Envelope *e1;
    Envelope *e2;
    std::list <CB>::iterator iter1;
    int nprocs;
    std::string comm;
    std::list <CB>::iterator iter_end;
    std::list <CB>::iterator iter;
    std::list <CB>::iterator iter3;

    iter_end = blist.end();

    for (iter = blist.begin (); iter != iter_end; iter++) {
        e1 = GetTransition (*iter)->GetEnvelope ();
        nprocs = e1->nprocs;
        comm = e1->comm;
        for (iter1 = blist.begin (); iter1 != iter_end; iter1++) {
            e2 = GetTransition (*iter1)->GetEnvelope ();
            if (e2->comm == e1->comm) {
                flist.push_back (CB(*iter1));
            }
        }
        if ((collective == BCAST || collective == GATHER ||
             collective == SCATTER || collective == SCATTERV ||
             collective == GATHERV || collective == REDUCE) && !flist.empty()) {
            std::list <CB>::iterator iter;
            std::list <CB>::iterator iter_end2 = flist.end();

            int root =
                GetTransition (*flist.begin())->GetEnvelope ()->count;
            for (iter = flist.begin (); iter != iter_end2; iter++) {
                int root1 = GetTransition (*iter)->GetEnvelope ()->count;

                if (root != root1) {
                    //FreeMemory(flist);
                    flist.clear ();
                    //FreeMemory(blist);
                    return false;
                }
            }
        }
 
        if (collective == COMM_CREATE || collective == COMM_DUP ||
            collective == CART_CREATE || collective == COMM_SPLIT) {
            /* 0 = COMM_WORLD, 1 = COMM_SELF, 2 = COMM_NULL */
            static int comm_id = 3;
            if (collective == COMM_SPLIT) {
                std::map<int, int> colorcount;

                /* Mark which colors are being used in the map. */
                std::list <CB>::iterator iter;
                std::list <CB>::iterator iter_end2 = flist.end();
                for (iter = flist.begin (); iter != iter_end2; iter++) {
                    colorcount[GetTransition (*iter)->GetEnvelope ()->comm_split_color] = 1;
                }

                /* Assign comm_id's to all of the new communicators. */
                std::map <int, int>::iterator iter_map;
                std::map <int, int>::iterator iter_map_end = colorcount.end();
                for (iter_map = colorcount.begin (); iter_map != iter_map_end; iter_map++) {
                    iter_map->second = comm_id++;
                }

                /* Put the new IDs in the envelopes. */
                for (iter = flist.begin (); iter != iter_end2; iter++) {
                    Envelope *e = GetTransition (*iter)->GetEnvelope ();
                    e->comm_id = colorcount[e->comm_split_color];
                }
            } else {
                int id = comm_id++;

                std::list <CB>::iterator iter;
                std::list <CB>::iterator iter_end2 = flist.end();
                for (iter = flist.begin (); iter != iter_end2; iter++) {
                    GetTransition(*iter)->GetEnvelope()->comm_id = id;
                }
            }
        }

        if ((int)flist.size () == nprocs) {
            break;
        } else {
            flist.clear ();
        }
    }

    if (! flist.empty ()) {
        ample_set.push_back (flist);
        return true;
    }
    return false;
}

bool Node::getNonWildcardReceive (std::vector <std::list <int> > &l) {
    for (int i = 0; i < NumProcs (); i++) {
        std::list <int>::iterator iter;
        std::list <int>::iterator iter_end;
        
        iter_end= l[i].end();
        for (iter = l[i].begin (); iter != iter_end; iter++) {
            
            if (GetTransition (i, *iter)->GetEnvelope ()->isRecvType ()) {
                if (GetTransition (i, *iter)->GetEnvelope ()->src != WILDCARD) {
                    CB tempCB(i, *iter);
                    CB c1;
                    if(getMatchingSend (c1, l, tempCB)) {
                        std::list <CB> ml;
                        ml.push_back (c1);
                        ml.push_back (CB(i, *iter));
                        ample_set.push_back (ml);
                        return true;
                    }
                }
            }
        }
    }
    return false;
}

/* The use of reverse_iterator here is necessary to preserve program-order
 * matching - This is based on the fact that GetEnabledTransistions also
 * uses a reverse iterator */
bool Node::getMatchingSend (CB &res, std::vector <std::list <int> > &l, CB &c) {
    int src = GetTransition(c)->GetEnvelope ()->src;
    std::list <int>::reverse_iterator iter;
    std::list <int>::reverse_iterator iter_end = l[src].rend();
    Envelope *e;
    Envelope *c_env = GetTransition(c)->GetEnvelope();
    for (iter = l[src].rbegin (); iter != iter_end; iter++) {
        e = GetTransition(src, (*iter))->GetEnvelope();

        if (e->isSendType () &&
            e->dest == c._pid &&
            e->comm == c_env->comm &&
            (e->stag == c_env->rtag || c_env->rtag == WILDCARD)) {
            res._pid = src;
            res._index = *iter;
            return true;
        }
    }
    return false;
}

bool Node::getAllMatchingSends (std::vector <std::list <int> > &l, CB &c, 
                                std::vector <std::list <CB> >& ms) {
    std::list <int>::reverse_iterator iter;
    std::list <int>::reverse_iterator iter_end;
    Envelope *e;
    Envelope *c_env = GetTransition(c)->GetEnvelope();
    bool found_ample = false;
    std::set<CB> *sends = &itree->matched_sends[c];

    for (int i = 0; i < NumProcs (); i++) {

        iter_end = l[i].rend();
        for (iter = l[i].rbegin (); iter != iter_end; iter++) {
            e = GetTransition(i, *iter)->GetEnvelope();

            if (e->isSendType () &&
                    e->dest == c._pid &&
                    e->comm == c_env->comm &&
                    (e->stag == c_env->rtag || c_env->rtag == WILDCARD) && (
#ifdef CONFIG_OPTIONAL_AMPLE_SET_FIX
                    Scheduler::_no_ample_set_fix ? true :
#endif
                          (sends->find(CB(i, *iter)) == sends->end()))) {
                std::list <CB> ml;
                ml.push_back (CB(i, *iter));
                ml.push_back (c);
                ms.push_back (ml);
                found_ample = true;
                break;
            }
        }
    }
    return found_ample;
}

void Node::GetallSends (std::vector <std::list <int> > &l) {
    bool first = false;

    for (int i = 0; i < NumProcs (); i++) {
        std::list <int>::iterator iter;
        std::list <int>::iterator iter_end;

        iter_end = l[i].end();

        for (iter = l[i].begin (); iter != iter_end; iter++) {
            if (GetTransition (i, (*iter))->GetEnvelope ()->isRecvType ()) {
                if (GetTransition (i, (*iter))->GetEnvelope ()->src == WILDCARD) {
                    CB tempCB(i, *iter);
                    if (getAllMatchingSends (l, tempCB, 
#ifdef CONFIG_OPTIONAL_AMPLE_SET_FIX
                            Scheduler::_no_ample_set_fix ? ample_set :
#endif
                            (!first) ? ample_set:other_wc_matches))  {
                        first = true;
                    }
                }
            }
        }
    }
    other_wc_matches.clear();
}

void Node::GetReceiveAmple (std::vector <std::list <int> > &l) {
    if (getNonWildcardReceive (l)) {
        return;
    }
    
    GetallSends (l);
}

bool Node::GetAmpleSet () {
    if (! ample_set.empty ()) {
        return true;
    }

    GetEnabledTransitions (enabled_transitions);
    /*
     * See if there is a Barrier set ready to go!
     */
    GetCollectiveAmple (enabled_transitions, BARRIER);
    if (! ample_set.empty ())
        return true;

    GetCollectiveAmple (enabled_transitions, BCAST);
    if (! ample_set.empty ())
        return true;


    GetCollectiveAmple (enabled_transitions, SCATTER);
    if (! ample_set.empty ())
        return true;

    GetCollectiveAmple (enabled_transitions, GATHER);
    if (! ample_set.empty ())
        return true;

    GetCollectiveAmple (enabled_transitions, SCATTERV);
    if (! ample_set.empty ())
        return true;

    GetCollectiveAmple (enabled_transitions, GATHERV);
    if (! ample_set.empty ())
        return true;

    GetCollectiveAmple (enabled_transitions, ALLGATHER);
    if (! ample_set.empty ())
        return true;

    GetCollectiveAmple (enabled_transitions, ALLGATHERV);
    if (! ample_set.empty ())
        return true;

    GetCollectiveAmple (enabled_transitions, ALLTOALL);
    if (! ample_set.empty ())
        return true;

    GetCollectiveAmple (enabled_transitions, ALLTOALLV);
    if (! ample_set.empty ())
        return true;

    GetCollectiveAmple (enabled_transitions, SCAN);
    if (! ample_set.empty ())
        return true;

    GetCollectiveAmple (enabled_transitions, EXSCAN);
    if (! ample_set.empty ())
        return true;

    GetCollectiveAmple (enabled_transitions, REDUCE);
    if (! ample_set.empty ())
        return true;

    GetCollectiveAmple (enabled_transitions, REDUCE_SCATTER);
    if (! ample_set.empty ())
        return true;

    GetCollectiveAmple (enabled_transitions, ALLREDUCE);
    if (! ample_set.empty ())
        return true;

    GetCollectiveAmple (enabled_transitions, FINALIZE);
    if (! ample_set.empty ())
        return true;

    GetCollectiveAmple (enabled_transitions, CART_CREATE);
    if (! ample_set.empty ())
        return true;

    GetCollectiveAmple (enabled_transitions, COMM_CREATE);
    if (! ample_set.empty ())
        return true;

    GetCollectiveAmple (enabled_transitions, COMM_DUP);
    if (! ample_set.empty ())
        return true;

    GetCollectiveAmple (enabled_transitions, COMM_SPLIT);
    if (! ample_set.empty ())
        return true;

    GetCollectiveAmple (enabled_transitions, COMM_FREE);
    if (! ample_set.empty ())
        return true;

    GetWaitorTestAmple (enabled_transitions);
    if (! ample_set.empty ()) {
        return true;
    }
    GetReceiveAmple (enabled_transitions);
    if (! ample_set.empty ())
        return true;

    /* Special case for Test & Iprobe calls 
     * If no call can progress and 
     * If there's a test call then we can match it and return false. 
     * Also need to remove the CB edges */

    std::list<CB> test_list;
    for (int i = 0; i < NumProcs(); i++) {
        Transition t = _tlist[i]->_tlist.back();
        if (t.GetEnvelope()->func_id == TEST ||
            t.GetEnvelope()->func_id == TESTALL ||
            t.GetEnvelope()->func_id == TESTANY ||
            t.GetEnvelope()->func_id == IPROBE) {
            test_list.push_back(CB (i, (int)_tlist[i]->_tlist.size()-1));

            //Need to clean up the CB edge here
            //Each of this transition's ancestors will have an edge
            //to each of the transition's descendants
            std::vector<int>::iterator iter = t.get_ancestors().begin();
            std::vector<int>::iterator iter_end = t.get_ancestors().end();
            for (; iter != iter_end; iter++) {
                std::vector<CB>::iterator iter2 = t.get_intra_cb().begin();
                std::vector<CB>::iterator iter2_end = t.get_intra_cb().end();
                for (; iter2 != iter2_end; iter2++) {
                    Transition* descendant = GetTransition(*iter2);
                    Transition* ancestor = GetTransition(i, *iter);
                    descendant->mod_ancestors().push_back(*iter);
                    ancestor->AddIntraCB(*iter2);
                }
            }
        }
    }
    if (!test_list.empty()) {
        ample_set.push_back(test_list);
    }
    if (!ample_set.empty())
        return true;

    
    return false;
}

Transition *Node::GetTransition (CB &c) {
    return GetTransition(c._pid, c._index);
}

Transition *Node::GetTransition (int pid, int index) {
    return &_tlist[pid]->_tlist[index];
}
