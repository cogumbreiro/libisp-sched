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
 * File:        InterleavingTree.cpp
 * Description: Implements ISP's POE algorithm
 * Contact:     isp-dev@cs.utah.edu
 */

#include <sstream>
#include <map>
#include <queue>
#include <string.h>

#include "InterleavingTree.hpp"
#include "Node.hpp"

ITree::ITree (Node *n, std::string name) {

    pname = name;
    _slist.push_back (n);
    depth = 0;
    have_wildcard = false;
#ifdef CONFIG_BOUNDED_MIXING
    expanded = 0;
#endif
    for (int i = 0; i < n->NumProcs(); i ++) {
        is_matched.push_back (new bool [MAX_TRANSITIONS]());
        is_issued.push_back(new bool [MAX_TRANSITIONS]());
    }
    last_matched = new int[n->NumProcs()]();
    for (int j = 0; j < n->NumProcs(); j++)
        last_matched[j] = -1;

    n->setITree(this);
}

void ITree::ResetMatchingInfo() {
    for (int i = 0; i< GetCurrNode()->NumProcs(); i++) {
        delete [] is_matched[i];
        delete [] is_issued[i];
        is_matched[i] = new bool[MAX_TRANSITIONS]();
        is_issued[i] = new bool[MAX_TRANSITIONS]();

    }
    delete [] last_matched;
    last_matched = new int[GetCurrNode()->NumProcs()]();
    for (int j = 0; j < GetCurrNode()->NumProcs(); j++)
        last_matched[j] = -1;
}

Node *ITree::GetCurrNode () {
    assert((int)depth < (int)_slist.size());
    return _slist[depth];
}

int ITree::CHECK (/*ServerSocket &sock, */std::list <int> &l) {
    bool probe_flag = false;
    int choice = 0; // default is EXP_MODE_LEFT_MOST
    Node *n = GetCurrNode();
    Envelope *env;
    if (n->GetAmpleSet()) {
      std::list<CB> cbl;

        // modify the ample set
        if (Scheduler::_fprs) {
            std::vector< std::list<CB> > new_ample_set;
            std::vector< std::list<CB> > dont_care_set;
            std::vector< std::list<CB> >::iterator as_iter;
            Envelope *this_env;
            for (as_iter = n->ample_set.begin() ; as_iter != n->ample_set.end() ; as_iter++) {
            	bool dont_care = true;
            	std::list<CB>::iterator cbiter;
            	for (cbiter = (*as_iter).begin() ; cbiter != (*as_iter).end() ; cbiter++) {
            		this_env = n->GetTransition(*cbiter)->GetEnvelope();
            		if (this_env->in_exall) {
            			new_ample_set.push_back(*as_iter);
			            dont_care = false;
            			break;
		            }
            	}

            	if (dont_care) {
            		dont_care_set.push_back(*as_iter);
            	}
            }
            int choice_dc = 0;
            if (dont_care_set.size() > 1) choice_dc = rand() % dont_care_set.size();
            if (dont_care_set.size() > 0) new_ample_set.push_back(dont_care_set.at(choice_dc));

            n->ample_set.clear();
            n->ample_set = new_ample_set;

            cbl = n->ample_set.at(choice);
        }
        else {
            if (n->ample_set.size() > 1 && Scheduler::_explore_mode == EXP_MODE_RANDOM) {
                choice = rand() % n->ample_set.size();
            }
            cbl = n->ample_set.at(choice);
        }

        if (depth >= (int)_slist.size ()-1) {
            Node *n_ = new Node(*n);
            _slist.push_back(n_);
            n->type = GENERAL_NODE;
            if(cbl.size() == 2) {
                env = n->GetTransition(cbl.back())->GetEnvelope();
                if ((env->func_id == RECV || env->func_id == IRECV) && env->src == WILDCARD) {
                    have_wildcard = true;
                    n->type = WILDCARD_RECV_NODE;
                    n->wildcard._pid = env->id;
                    n->wildcard._index = env->index;
                } else if (env->func_id == PROBE && env->src == WILDCARD) {
                    have_wildcard = true;
                    n->type = WILDCARD_PROBE_NODE;
                    n->wildcard._pid = env->id;
                    n->wildcard._index = env->index;
                }
            }
        } else {
            if(n->isWildcardNode()) {
                env = n->GetTransition(cbl.back())->GetEnvelope();
                n->wildcard._pid = env->id;
                n->wildcard._index = env->index;
            }
        }
        depth++;
        n = GetCurrNode();

        /* If the match set has a send and receive - the send is the first item, the receive second */
        /* Here we get the source of the send, so that we can rewrite the wildcard */
        int source = cbl.front ()._pid;

        if (n->GetTransition(cbl.front())->GetEnvelope ()->isSendType ()) {
            /* if it's a probe call, we don't issue the send, so no
               need to update the matching of the send */
            if (n->GetTransition(cbl.back())->GetEnvelope ()->func_id != PROBE &&
                n->GetTransition(cbl.back())->GetEnvelope ()->func_id != IPROBE) {
                n->GetTransition(cbl.front())->set_curr_matching(cbl.back());
            } else {
                probe_flag = true;
                Scheduler::_probed = true;
            }

            n->GetTransition(cbl.back())->set_curr_matching(cbl.front());
        }

        //Check if the data types match CGD
        if (n->GetTransition(cbl.front())->GetEnvelope()->isSendType ()){
					int currType = n->GetTransition(cbl.front())->GetEnvelope()->data_type;
					int matchType = n->GetTransition(cbl.back())->GetEnvelope()->data_type;
					if(currType == matchType){
						n->GetTransition(cbl.front())->GetEnvelope()->typesMatch = true;
						n->GetTransition(cbl.back())->GetEnvelope()->typesMatch = true;
					}else{
						n->GetTransition(cbl.front())->GetEnvelope()->typesMatch = false;
						n->GetTransition(cbl.back())->GetEnvelope()->typesMatch = false;

						Envelope *env = n->GetTransition(cbl.front())->GetEnvelope ();
						Scheduler::_mismatchLog << env->filename << " "
											    << env->linenumber << std::endl;
						env = n->GetTransition(cbl.back())->GetEnvelope ();
						Scheduler::_mismatchLog << env->filename << " "
										        << env->linenumber << std::endl;
					}
        }
        else if(n->GetTransition(cbl.front())->GetEnvelope()->data_type == -1){
        	Envelope *env = n->GetTransition(cbl.front())->GetEnvelope();
        	Scheduler::_mismatchLog << env->filename << " "
						            << env->linenumber << std::endl;
        }

        std::list <CB>::iterator iter;
        std::list <CB>::iterator iter_end = cbl.end();
        bool test_iprobe_calls_only = true;
        for (iter = cbl.begin (); iter != iter_end; iter++) {
            env = n->GetTransition (*iter)->GetEnvelope ();

            if (env->func_id != IPROBE && env->func_id != TEST &&
                env->func_id != TESTALL && env->func_id != TESTANY) {
                test_iprobe_calls_only = false;
                break;
            }

        }

        for (iter = cbl.begin (); iter != iter_end; iter++) {

            env = n->GetTransition (*iter)->GetEnvelope ();

            if (test_iprobe_calls_only) {
                source = -1;
            }
            /* If the other call is a probe, we only issue the send
             * but leave the send un-matched! */
            if (!probe_flag || !env->isSendType()) {
                is_matched[iter->_pid][iter->_index] = true;
                n->_tlist[iter->_pid]->_ulist.remove(iter->_index);
            }

            /* increase the last matched transition to the latest matched transition */

            int temp_last_matched = last_matched[iter->_pid];
            while (temp_last_matched < (int) (n->_tlist[iter->_pid]->_tlist.size()-1)
                   && is_matched[iter->_pid][temp_last_matched + 1]) {
                temp_last_matched ++;
            }
            last_matched[iter->_pid] = temp_last_matched;

            /* if the call has been issued earlier, do not issue it again */
            /* What happens here: If we issue a pair of Send/Probe, do
             * not allow the scheduler to read the next envelope of
             * the Send process. If we issue a pair of Send/Recv, allow the
             * scheduler to read the next envelope of the send process */

            if (is_issued[iter->_pid][iter->_index]) {
                if (env->isSendType() && env->isBlockingType() && !Scheduler::_probed) {
                    Scheduler::_runQ[iter->_pid]->_read_next_env = true;
                }
                continue;
            }
            std::ostringstream oss;

            oss << goahead << " " << env->index << " " << source;

            env->Issued ();
            if (env->func_id == COMM_CREATE || env->func_id == COMM_DUP
                || env->func_id == CART_CREATE || env->func_id == COMM_SPLIT) {
                oss << " " << env->comm_id;
            } else {
                oss << " " << 0;
            }

            if (env-> isBlockingType ()) {
                oss << " " << 2 ;
                l.push_back (iter->_pid);
            } else {
                oss << " " << 1 ;
            }
            sock.Send (iter->_pid, oss.str());
            is_issued[iter->_pid][iter->_index]= true;
        }
    } else {
        sock.ExitMpiProcessAndWait (true);
        Scheduler::_just_dead_lock = true;
        //exit (1);
        return 1;
    }
    return 0;
}

/* This method is called at the end of each interleaving
 * The following task is accomplished:
 * Add the interCB edges to all the nodes
 * Find out the sends for which buffering would make it illegible
 * to match with some wildcard receives.
 * Side effects: interCB edges are added, ample set will be changed
 * if the sends could become a potential match due to buffering
 */
void ITree::ProcessInterleaving () {
    /* Only do this if we have seen a wildcard receive before
     * Otherwise we'll just kill performance!!! */
    if (have_wildcard) {
        /* Go back to each transition and add interCB edges
         * Note that we could not do this on the fly as we had not
         * seen all the possible matchings */
        AddInterCB ();

        /* Find all the sends that could have been co-enabled by buffering */
        FindCoEnabledSends ();
    }
}

/* Precond: c has to be of type Wait or Test
 * Return a CB edge that points to a send
 * if this Wait/Waitall is for a send request, return NULL otherwise.
 * Whichever method that calls this will have
 * to make sure to clean up this returned edge to avoid memory leak */
bool ITree::findSendOfThisWait (CB &res, CB &c) {
    Envelope *e = GetCurrNode ()->GetTransition (c)->GetEnvelope ();
    if (e->isWaitorTestType ()) {
        for (unsigned int i = 0 ; i < e->req_procs.size (); i++) {
            CB wc (c._pid, e->req_procs[i]);
            if (GetCurrNode()->GetTransition(wc)->GetEnvelope()->isSendType()) {
                res = wc;
                return true;
            }
        }
    }
    return false;
}

/* going through all the tlist, return the max number of MPI calls
 * Side effects: None */
size_t ITree::getMaxTlistSize() {
    size_t ret = GetCurrNode()->_tlist[0]->_tlist.size();
    for (size_t i = 1; i < GetCurrNode()->_tlist.size(); i++) {
        if (ret < GetCurrNode()->_tlist[i]->_tlist.size())
            ret = GetCurrNode()->_tlist[i]->_tlist.size();
    }
    return ret;
}

/* Return true if there exists a path from src to destination
 * We do not consider the wait node's since the sends are buffered
 * Side effects: visited array is used but not cleaned up.
 * The expectation is that whichever method that uses visited will
 * have to initialized it.
 * We're using BFS here, because DFS would result in a very deep
 * recursion.
 */
bool ITree::FindNonSendWaitPath (bool ** visited, CB &src, CB &dest) {
    if (src == dest)
        return true;
    if (src._pid == dest._pid && src._index > dest._index)
        return false;

    std::vector <CB>::iterator iter;
    std::vector <CB>::iterator iter_end;
    std::queue <CB> worklist;
    CB current(0,0);

    /* initialize the visited array */
    for (int i = 0; i < GetCurrNode()->NumProcs(); i++) {
        for (size_t j = 0; j < getMaxTlistSize(); j ++)
            visited[i][j] = false;
    }


    /* doing a BFS search */
    worklist.push(src);
    while (!worklist.empty()) {
        CB c;
        current = worklist.front();
        worklist.pop();
        if (current == dest) {
            return true;
        }
        visited[current._pid][current._index]= true;
        /* now add all the intraCB edges of current into the worklist */
        Transition t = *GetCurrNode()->GetTransition(current);
        iter = t.get_intra_cb().begin();
        iter_end = t.get_intra_cb().end();
        for (; iter != iter_end; iter++) {
            if (findSendOfThisWait(c, *iter)) {
                visited[iter->_pid][iter->_index] = true;
            }
            if (!visited[iter->_pid][iter->_index]) {
                worklist.push(*iter);
                visited[iter->_pid][iter->_index] = true;

            }
        }
        /* now add all the interCB edges of current into the worklist */
        iter = t.get_inter_cb().begin();
        iter_end = t.get_inter_cb().end();
        for (; iter != iter_end; iter++) {
            if (findSendOfThisWait(c, *iter)) {
                //delete c;
                visited[iter->_pid][iter->_index] = true;
            }

            if (!visited[iter->_pid][iter->_index]) {
                worklist.push(*iter);
                visited[iter->_pid][iter->_index] = true;

            }
        }
    }
    return false;

}

void ITree::FindCoEnabledSends () {
    std::vector <Node *>::iterator iter;
    std::vector <Node *>::iterator iter_end = _slist.end ();

    /* keep visited CB edges here
     * This array will be used by FindNonSenWait
     * We don't want to alloc/re-alloc too many times
     */
    bool** visited = new bool* [GetCurrNode()->NumProcs()];
    for (int i = 0; i < GetCurrNode()->NumProcs(); i++) {
        visited[i] = new bool [getMaxTlistSize()];
    }

    for (iter = _slist.begin (); iter != iter_end-1; iter++) {
        Node *n = (*iter);

        /* Only care about nodes with wild card receive */
        if (!n->isWildcardNode()) continue;

        CB send = n->ample_set.front().front();

        std::set <CB> *prev_sends = &matched_sends[n->wildcard];
        prev_sends->insert(send);
        std::map <CB, std::list <CB> >::iterator csi = aux_coenabled_sends.find(n->wildcard);
        if(csi != aux_coenabled_sends.end())
            csi->second.remove(send);

        Envelope *renv = GetCurrNode ()->GetTransition ((n->wildcard))->GetEnvelope ();
        for (int i = 0 ; i < n->NumProcs (); i++) {
            if (i == n->wildcard._pid) {
                continue;
            }
            size_t tlist_size = GetCurrNode()->_tlist[i]->_tlist.size();
            for (unsigned int j = 0 ; j < tlist_size; j++) {
                Envelope *e = GetCurrNode ()->_tlist[i]->_tlist[j].GetEnvelope ();
                if (! e->isSendType ()) {
                    continue;
                }
                if (e->dest == n->wildcard._pid &&
                    (e->stag == renv->rtag || renv->rtag == WILDCARD) &&
                    e->comm == renv->comm) {
                    CB c(i, e->index);
                    std::list <int>::iterator it;
                    std::list <int>::iterator it_end = n->enabled_transitions[i].end();
                    bool sendcoenabled = false;
                    for (it = n->enabled_transitions[i].begin (); it != it_end; it++) {
                        if (*it >= e->index )  {
                            sendcoenabled = true;
                            break;
                        }
                    }
                    if (!sendcoenabled) {
                        bool overtaking = (c._pid ==
                                GetCurrNode()->GetTransition(n->wildcard)->
                                get_curr_matching()._pid);

                        if (overtaking ||
                            FindNonSendWaitPath (visited, n->wildcard, c)) {
                            break;
                        } else {
                            if(prev_sends->find(c) == prev_sends->end()) {
                                n->has_aux_coenabled_sends = true;
                                std::list <CB> *sends = &aux_coenabled_sends[n->wildcard];
                                std::list <CB>::iterator si = sends->begin();
                                std::list <CB>::iterator sie = sends->end();
                                while(si != sie && *si < c) si++;
                                if(si == sie || *si != c) sends->insert(si, c);
                            }
                        }
                    }
                }
            }
        }
    }
    /* freeing memory */
    for (int i = 0; i < GetCurrNode()->NumProcs(); i++) {
        delete [] visited[i];
    }
    delete [] visited;
}

void ITree::ClearInterCB () {
    Node *n = GetCurrNode ();

    for (int i = 0 ; i < n->NumProcs (); i++) {
        for (unsigned int j = 0;  j < n->_tlist[i]->_tlist.size(); j++) {
            std::vector<CB>::iterator iter;
            n->_tlist[i]->_tlist[j].mod_inter_cb().clear();
        }
    }
}

void ITree::AddInterCB () {
    std::vector <Node *>::iterator iter;
    std::vector <Node *>::iterator iter_end = _slist.end ();

    for (iter = _slist.begin(); iter != iter_end; iter++) {
        if ((*iter)->ample_set.size() == 0) {
            continue;
        }
        std::list <CB> mset = (*iter)->ample_set.front();

        if (mset.size() <= 1) {
            continue;
        }
        if (mset.size() == 2) {
            Envelope *e = (*iter)->GetTransition (mset.back())->GetEnvelope ();
            if ((e->func_id == IRECV || e->func_id == RECV) &&
                                                    e->src == WILDCARD) {
                GetCurrNode()->GetTransition(mset.back())->AddInterCB(mset.front());
                continue;
            }
        }
        std::list <CB>::iterator it1;
        std::list <CB>::iterator it_end = mset.end();
        bool only_sends = true;
        for (it1 = mset.begin(); it1 != it_end ; it1++) {
            if (! (*iter)->GetTransition(*it1)->GetEnvelope()->isSendType())
                only_sends = false;
        }
        if (only_sends)
            continue;

        for (it1 = mset.begin(); it1 != it_end ; it1++) {
            std::list <CB>::iterator it2;
            std::list <CB>::iterator it2_end = mset.end();
            for (it2 = mset.begin(); it2 != it2_end ; it2++) {
                if (it2 != it1) {
                    GetCurrNode()->GetTransition(*it1)->AddInterCB(*it2);

                }
            }
        }
    }
}

/* This looks complicated, but the algorithm is as follows: Go through
 * all the interleaving nodes, delete the first match-set of each node
 * This is what we've covered so far.  If the node is empty after
 * this, simply remove that node.  If there is one node that has more
 * than one match sets, which means we have to cover another interleaving
 */
bool ITree::NextInterleaving () {
    ClearInterCB();

    int i = depth+1;
    assert(i == (int) _slist.size());

    int oldDepth = depth;
    last_node = GetCurrNode ();
    if (Scheduler::_fprs) {
	    if (Scheduler::_explore_mode == EXP_MODE_RANDOM) return true;
	    else if (Scheduler::_explore_mode == EXP_MODE_LEFT_MOST) return false;
    }
    else {
        if (Scheduler::_explore_mode == EXP_MODE_RANDOM ||Scheduler::_explore_mode == EXP_MODE_LEFT_MOST)
            return false;
    }

    while (i-- > 0) {
        /* erase the first match-set */
        Node *n = _slist[i];
        if (!n->ample_set.empty()) {
            n->ample_set.erase(n->ample_set.begin());
        }

        if (!( n->ample_set.empty() && (
#ifdef CONFIG_OPTIONAL_AMPLE_SET_FIX
                Scheduler::_no_ample_set_fix ? true :
#endif
                    (!n->isWildcardNode() ||
                    (n->isWildcardNode() && aux_coenabled_sends[n->wildcard].empty()))))) {

#ifdef CONFIG_BOUNDED_MIXING
            if (n->isWildcardNode() && Scheduler::_bound &&
                    !n->expand && expanded < Scheduler::_bound) {
                n->expand = true;
                expanded++;
            }
            if(Scheduler::_bound && n->expand || !Scheduler::_bound) {
#endif
                delete last_node;
                return true;
#ifdef CONFIG_BOUNDED_MIXING
            }
#endif
        }

        /* if ample_set is empty, delete that node */
        if(n->isWildcardNode()) {
#ifdef CONFIG_BOUNDED_MIXING
            if(Scheduler::_bound && n->expand) {
                expanded--;
            }
#endif
            matched_sends[n->wildcard].clear();
        }
        if (depth != oldDepth) {
            delete *(_slist.end()-1);
        }
#endif
        _slist.pop_back ();
        depth--;
        i = (int)_slist.size();
    }
    return false;
}

void ITree::resetDepth () {
    depth = 0;
    for (size_t i = 0; i < _slist[depth]->_tlist.size(); i ++) {
        _slist[depth]->_tlist[i]->last_matched = -1;
    }
}
