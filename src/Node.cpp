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
#include <boost/range/adaptor/reversed.hpp>

#include "Node.hpp"

using boost::adaptors::reverse;
using std::list;
using std::vector;
/*
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
*/
/*
void Node::deepCopy() {
    int i;
    vector <TransitionList*> tlist;
    for(i=0; i<_num_procs; i++) {
        TransitionList *tl = new TransitionList(*(this->_tlist[i]));
        tlist.push_back(tl);
    }
    _tlist = tlist;
    tlist_dealloc = true;
}
*/
int Node::getTotalMpiCalls() const {
    int sum = 0;
    for (auto & trans : _tlist) {
      sum += trans->size();
    }
    return sum;
}

bool Node::allAncestorsMatched (const CB handle, const vector <int> &indices) const {
    bool is_wait_or_test_type = getTransition(handle).getEnvelope().isWaitorTestType();
    auto pid = handle.pid;
    auto & pid_transitions = _tlist[pid];
    for (auto curr : indices) {
        auto curr_handle = CB(pid, curr);
        auto & curr_trans = getTransition(curr_handle);
        auto curr_env = curr_trans.getEnvelope();
        auto curr_func = curr_env.func_id;
        if (!matcher.isMatched(curr_handle)) {
            // Wei-Fan Chiang: I don't know why I need to add this line. But, I need it........
            if (curr_func == PCONTROL) {
                continue;
            }
            if (is_wait_or_test_type && /* !Scheduler::_send_block*/
                    curr_func == ISEND) {
                continue;
            }
            return false;
        } else if (curr_func == WAIT ||
                   curr_func == TEST ||
                   curr_func == WAITALL ||
                   curr_func == TESTALL) {

            for (auto anc_id : curr_trans.getAncestors()) {
                auto anc = CB(pid, anc_id);
                auto & anc_env = getTransition(anc).getEnvelope();
                if (!matcher.isMatched(anc) && curr_env.matchSend(anc_env)) {
                    return false;
                }
            }
        }
    }
    return true;
}

bool Node::anyAncestorMatched(const CB handle, const vector<int> &ops) const {
    bool any_match = false;

    auto env = getTransition(handle).getEnvelope ();
    bool is_wait_or_test_type = env.isWaitorTestType();
    if (is_wait_or_test_type && ops.size() == 0) {
        return true;
    }
    vector<int> filtered = ops;
    for (auto op : env.req_procs) {
        auto req = CB(handle.pid, op);
        if (matcher.isMatched(req) ||
                (is_wait_or_test_type && /*!Scheduler::_send_block &&*/
                    getTransition(req).getEnvelope().func_id == ISEND)) {
            any_match = true;
        }
        filtered.erase(std::remove(filtered.begin(), filtered.end(), op), filtered.end());
    }
    if (!any_match) {
        return false;
    }
    for (auto id : filtered) {
        auto op = CB(handle.pid, id);
        if(!matcher.isMatched(op)) {
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
 vector <list<int> > Node::createEnabledTransitions() const {
    vector <list <int> > result;

    for (int i = 0; i < getNumProcs(); i++) {
        result.push_back (list<int>());
    }

    for (int pid = 0 ; pid < getNumProcs(); pid++) {
        int op = _tlist[pid]->size() - 1;
        for (Transition & trans : *_tlist[pid]) {
            CB h(pid,op);
            if (!matcher.isMatched(h)) {
                vector<int> ancestors(getTransition(h).getAncestors());
                const auto func_name = trans.getEnvelope().func_id;
                if ((func_name != WAITANY && func_name != TESTANY) &&
                        allAncestorsMatched(h, ancestors)) {
                    result[pid].push_back(op);
                } else if ((func_name == WAITANY || func_name == TESTANY) &&
                        anyAncestorMatched(h, ancestors)) {
                    result[pid].push_back(op);
                }
            }
            op--;
            if (op <= matcher.findLastMatched(pid)) {
                break;
            }
        }
    }
    return result;
}

void Node::addWaitorTestAmple(const vector<list<int> > &indices) {
    list <CB> blist;
    for (int pid = 0; pid < getNumProcs(); pid++) {
        for (auto idx : indices[pid]) {
            CB op = CB(pid, idx);
            if (getTransition(op).getEnvelope().isWaitorTestType()) {
                blist.push_back(op);
            }
        }
    }
    if (blist.size() > 0) {
        ample_set.push_back(blist);
    }
}

bool Node::getCollectiveAmple (vector <list <int> > &l, int collective) {
    list <CB> blist;
    list <CB> flist;

    for (int i = 0; i < NumProcs (); i++) {
        list <int>::iterator iter;
        list <int>::iterator iter_end;

        iter_end = l[i].end();
        for (iter = l[i].begin (); iter != iter_end; iter++) {
            if (getTransition (i, *iter)->getEnvelope ()->func_id == collective) {
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
    list <CB>::iterator iter1;
    int nprocs;
    std::string comm;
    list <CB>::iterator iter_end;
    list <CB>::iterator iter;
    list <CB>::iterator iter3;

    iter_end = blist.end();

    for (iter = blist.begin (); iter != iter_end; iter++) {
        e1 = getTransition (*iter)->getEnvelope ();
        nprocs = e1->nprocs;
        comm = e1->comm;
        for (iter1 = blist.begin (); iter1 != iter_end; iter1++) {
            e2 = getTransition (*iter1)->getEnvelope ();
            if (e2->comm == e1->comm) {
                flist.push_back (CB(*iter1));
            }
        }
        if ((collective == BCAST || collective == GATHER ||
             collective == SCATTER || collective == SCATTERV ||
             collective == GATHERV || collective == REDUCE) && !flist.empty()) {
            list <CB>::iterator iter;
            list <CB>::iterator iter_end2 = flist.end();

            int root =
                getTransition (*flist.begin())->getEnvelope ()->count;
            for (iter = flist.begin (); iter != iter_end2; iter++) {
                int root1 = getTransition (*iter)->getEnvelope ()->count;

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
                list <CB>::iterator iter;
                list <CB>::iterator iter_end2 = flist.end();
                for (iter = flist.begin (); iter != iter_end2; iter++) {
                    colorcount[getTransition (*iter)->getEnvelope ()->comm_split_color] = 1;
                }

                /* Assign comm_id's to all of the new communicators. */
                std::map <int, int>::iterator iter_map;
                std::map <int, int>::iterator iter_map_end = colorcount.end();
                for (iter_map = colorcount.begin (); iter_map != iter_map_end; iter_map++) {
                    iter_map->second = comm_id++;
                }

                /* Put the new IDs in the envelopes. */
                for (iter = flist.begin (); iter != iter_end2; iter++) {
                    Envelope *e = getTransition (*iter)->getEnvelope ();
                    e->comm_id = colorcount[e->comm_split_color];
                }
            } else {
                int id = comm_id++;

                list <CB>::iterator iter;
                list <CB>::iterator iter_end2 = flist.end();
                for (iter = flist.begin (); iter != iter_end2; iter++) {
                    getTransition(*iter)->getEnvelope()->comm_id = id;
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

bool Node::getNonWildcardReceive (vector <list <int> > &l) {
    for (int i = 0; i < NumProcs (); i++) {
        list <int>::iterator iter;
        list <int>::iterator iter_end;

        iter_end= l[i].end();
        for (iter = l[i].begin (); iter != iter_end; iter++) {

            if (getTransition (i, *iter)->getEnvelope ()->isRecvType ()) {
                if (getTransition (i, *iter)->getEnvelope ()->src != WILDCARD) {
                    CB tempCB(i, *iter);
                    CB c1;
                    if(getMatchingSend (c1, l, tempCB)) {
                        list <CB> ml;
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
bool Node::getMatchingSend (CB &res, vector <list <int> > &l, CB &c) {
    int src = getTransition(c)->getEnvelope ()->src;
    list <int>::reverse_iterator iter;
    list <int>::reverse_iterator iter_end = l[src].rend();
    Envelope *e;
    Envelope *c_env = getTransition(c)->getEnvelope();
    for (iter = l[src].rbegin (); iter != iter_end; iter++) {
        e = getTransition(src, (*iter))->getEnvelope();

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

bool Node::getAllMatchingSends (vector <list <int> > &l, CB &c,
                                vector <list <CB> >& ms) {
    list <int>::reverse_iterator iter;
    list <int>::reverse_iterator iter_end;
    Envelope *e;
    Envelope *c_env = getTransition(c)->getEnvelope();
    bool found_ample = false;
    std::set<CB> *sends = &itree->matched_sends[c];

    for (int i = 0; i < NumProcs (); i++) {

        iter_end = l[i].rend();
        for (iter = l[i].rbegin (); iter != iter_end; iter++) {
            e = getTransition(i, *iter)->getEnvelope();

            if (e->isSendType () &&
                    e->dest == c._pid &&
                    e->comm == c_env->comm &&
                    (e->stag == c_env->rtag || c_env->rtag == WILDCARD) && (
#ifdef CONFIG_OPTIONAL_AMPLE_SET_FIX
                    Scheduler::_no_ample_set_fix ? true :
#endif
                          (sends->find(CB(i, *iter)) == sends->end()))) {
                list <CB> ml;
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

void Node::getallSends (vector <list <int> > &l) {
    bool first = false;

    for (int i = 0; i < NumProcs (); i++) {
        list <int>::iterator iter;
        list <int>::iterator iter_end;

        iter_end = l[i].end();

        for (iter = l[i].begin (); iter != iter_end; iter++) {
            if (getTransition (i, (*iter))->getEnvelope ()->isRecvType ()) {
                if (getTransition (i, (*iter))->getEnvelope ()->src == WILDCARD) {
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

void Node::getReceiveAmple (vector <list <int> > &l) {
    if (getNonWildcardReceive (l)) {
        return;
    }

    GetallSends (l);
}

bool Node::getAmpleSet () {
    if (! ample_set.empty ()) {
        return true;
    }

    getEnabledTransitions (enabled_transitions);
    /*
     * See if there is a Barrier set ready to go!
     */
    getCollectiveAmple (enabled_transitions, BARRIER);
    if (! ample_set.empty ())
        return true;

    getCollectiveAmple (enabled_transitions, BCAST);
    if (! ample_set.empty ())
        return true;


    getCollectiveAmple (enabled_transitions, SCATTER);
    if (! ample_set.empty ())
        return true;

    getCollectiveAmple (enabled_transitions, GATHER);
    if (! ample_set.empty ())
        return true;

    getCollectiveAmple (enabled_transitions, SCATTERV);
    if (! ample_set.empty ())
        return true;

    getCollectiveAmple (enabled_transitions, GATHERV);
    if (! ample_set.empty ())
        return true;

    getCollectiveAmple (enabled_transitions, ALLGATHER);
    if (! ample_set.empty ())
        return true;

    getCollectiveAmple (enabled_transitions, ALLGATHERV);
    if (! ample_set.empty ())
        return true;

    getCollectiveAmple (enabled_transitions, ALLTOALL);
    if (! ample_set.empty ())
        return true;

    getCollectiveAmple (enabled_transitions, ALLTOALLV);
    if (! ample_set.empty ())
        return true;

    getCollectiveAmple (enabled_transitions, SCAN);
    if (! ample_set.empty ())
        return true;

    getCollectiveAmple (enabled_transitions, EXSCAN);
    if (! ample_set.empty ())
        return true;

    getCollectiveAmple (enabled_transitions, REDUCE);
    if (! ample_set.empty ())
        return true;

    getCollectiveAmple (enabled_transitions, REDUCE_SCATTER);
    if (! ample_set.empty ())
        return true;

    getCollectiveAmple (enabled_transitions, ALLREDUCE);
    if (! ample_set.empty ())
        return true;

    getCollectiveAmple (enabled_transitions, FINALIZE);
    if (! ample_set.empty ())
        return true;

    getCollectiveAmple (enabled_transitions, CART_CREATE);
    if (! ample_set.empty ())
        return true;

    getCollectiveAmple (enabled_transitions, COMM_CREATE);
    if (! ample_set.empty ())
        return true;

    getCollectiveAmple (enabled_transitions, COMM_DUP);
    if (! ample_set.empty ())
        return true;

    getCollectiveAmple (enabled_transitions, COMM_SPLIT);
    if (! ample_set.empty ())
        return true;

    getCollectiveAmple (enabled_transitions, COMM_FREE);
    if (! ample_set.empty ())
        return true;

    getWaitorTestAmple (enabled_transitions);
    if (! ample_set.empty ()) {
        return true;
    }
    getReceiveAmple (enabled_transitions);
    if (! ample_set.empty ())
        return true;

    /* Special case for Test & Iprobe calls
     * If no call can progress and
     * If there's a test call then we can match it and return false.
     * Also need to remove the CB edges */

    list<CB> test_list;
    for (int i = 0; i < getNumProcs(); i++) {
        Transition t = _tlist[i]->_tlist.back();
        if (t.getEnvelope()->func_id == TEST ||
            t.getEnvelope()->func_id == TESTALL ||
            t.getEnvelope()->func_id == TESTANY ||
            t.getEnvelope()->func_id == IPROBE) {
            test_list.push_back(CB (i, (int)_tlist[i]->_tlist.size()-1));

            //Need to clean up the CB edge here
            //Each of this transition's ancestors will have an edge
            //to each of the transition's descendants
            vector<int>::iterator iter = t.getAncestors().begin();
            vector<int>::iterator iter_end = t.getAncestors().end();
            for (; iter != iter_end; iter++) {
                vector<CB>::iterator iter2 = t.get_intra_cb().begin();
                vector<CB>::iterator iter2_end = t.get_intra_cb().end();
                for (; iter2 != iter2_end; iter2++) {
                    Transition* descendant = getTransition(*iter2);
                    Transition* ancestor = getTransition(i, *iter);
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
