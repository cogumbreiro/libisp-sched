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
#include "Node.hpp"

using std::vector;
using boost::optional;

/*
ITree::ITree (Node *n, std::string name) {
    pname = name;
    _slist.push_back (n);
    depth = 0;
    have_wildcard = false;
    for (int i = 0; i < n->NumProcs(); i ++) {
        is_matched.push_back (new bool [MAX_TRANSITIONS]());
        is_issued.push_back(new bool [MAX_TRANSITIONS]());
    }
    last_matched = new int[n->NumProcs()]();
    for (int j = 0; j < n->NumProcs(); j++)
        last_matched[j] = -1;

    n->setITree(this);
}
*/
void ITree::ResetMatchingInfo() {
    for (int i = 0; i< GetCurrNode()->getNumProcs(); i++) {
        delete [] is_matched[i];
        delete [] is_issued[i];
        is_matched[i] = new bool[MAX_TRANSITIONS]();
        is_issued[i] = new bool[MAX_TRANSITIONS]();

    }
    delete [] last_matched;
    last_matched = new int[GetCurrNode()->getNumProcs()]();
    for (int j = 0; j < GetCurrNode()->getNumProcs(); j++)
        last_matched[j] = -1;
}

Node *ITree::GetCurrNode () const {
    assert((int)depth < (int)_slist.size());
    return _slist[depth];
}

int ITree::CHECK (/*ServerSocket &sock, */std::list <int> &l) {
    bool probe_flag = false;
    int choice = 0; // default is EXP_MODE_LEFT_MOST
    Node *n = GetCurrNode();
    //Envelope *env;
    auto ample_set = n->buildAmpleSet();
    if (ample_set.empty()) {
        // XXX: DEADLOCK!
        return 1;
    }
    auto cbl = ample_set.at(choice);
    if (depth >= (int)_slist.size ()-1) {
        // add a copy of the current node in `_slist'
        //XXX: Node *n_ = new Node(*n);
        //XXX: _slist.push_back(n_);
        n->type = GENERAL_NODE;
        if(cbl.size() == 2) {
            auto last_ptr = cbl.back();
            auto env = last_ptr->getEnvelope();
            if ((env.func_id == RECV || env.func_id == IRECV) && env.src == WILDCARD) {
                have_wildcard = true;
                n->type = WILDCARD_RECV_NODE;
                n->wildcard = last_ptr;
            } else if (env.func_id == PROBE && env.src == WILDCARD) {
                have_wildcard = true;
                n->type = WILDCARD_PROBE_NODE;
                n->wildcard = last_ptr;
            }
        }
    } else {
        if(n->isWildcardNode()) {
            n->wildcard = cbl.back();
        }
    }
    depth++;
    n = GetCurrNode();

    /* If the match set has a send and receive - the send is the first item, the receive second */
    /* Here we get the source of the send, so that we can rewrite the wildcard */
    auto front_ptr = cbl.front();
    auto front_env = front_ptr->getEnvelope();
    auto back_ptr = cbl.back();
    auto & back_env = back_ptr->getEnvelope();
    if (front_env.isSendType()) {
        /* if it's a probe call, we don't issue the send, so no
           need to update the matching of the send */
        if (back_env.func_id != PROBE &&
            back_env.func_id != IPROBE) {
            front_ptr->setCurrentMatching(back_ptr);
        } else {
            probe_flag = true;
        }
        back_ptr->setCurrentMatching(front_ptr);
    }

    //Check if the data types match CGD
    if (front_env.isSendType()){
        auto back_ptr = cbl.back();
        auto types_match = front_env.data_type == back_env.data_type;
        front_ptr->types_match = types_match;
        back_ptr->types_match = types_match;
        // XXX: log mismatch if mismatches
    } else if(front_env.data_type == -1){
        // XXX: log mismatch
    }

    bool test_iprobe_calls_only = true;
    for (auto curr : cbl) {
        auto & env = curr->getEnvelope();
        if (env.func_id != IPROBE && env.func_id != TEST &&
            env.func_id != TESTALL && env.func_id != TESTANY) {
            test_iprobe_calls_only = false;
            break;
        }

    }

    for (auto curr : cbl) {
        auto & env = curr->getEnvelope();

        if (test_iprobe_calls_only) {
            front_ptr = nullptr;
        }
        /* If the other call is a probe, we only issue the send
         * but leave the send un-matched! */
        if (!probe_flag || !env.isSendType()) {
            n->getState().setMatched(curr);
        }
        curr->setIssued();
        #if 0
        /* if the call has been issued earlier, do not issue it again */
        /* What happens here: If we issue a pair of Send/Probe, do
         * not allow the scheduler to read the next envelope of
         * the Send process. If we issue a pair of Send/Recv, allow the
         * scheduler to read the next envelope of the send process */
        if (curr->isIssued()) {
            if (env.isSendType() && env.isBlockingType() /*XXX: && !Scheduler::_probed*/) {
                Scheduler::_runQ[iter->_pid]->_read_next_env = true;
            }
            continue;
        }
        std::ostringstream oss;

        oss << goahead << " " << env->index << " " << source;

        //XXX: env->issue();
        if (env.func_id == COMM_CREATE || env.func_id == COMM_DUP
            || env.func_id == CART_CREATE || env.func_id == COMM_SPLIT) {
            oss << " " << env->comm_id;
        } else {
            oss << " " << 0;
        }

        if (env.isBlockingType()) {
            oss << " " << 2 ;
            l.push_back(curr->pid);
        } else {
            oss << " " << 1 ;
        }
        sock.Send(curr->pid, oss.str());
        curr.setIssued();
        #endif
    }
    return 0;
}

void ITree::AddInterCB () {
    vector <Node *>::iterator iter;
    vector <Node *>::iterator iter_end = _slist.end ();

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

/* Precond: wait has to be of type Wait or Test
 * Return a transition that points to a send
 * if this Wait/Waitall is for a send request, return NULL otherwise.
 * Whichever method that calls this will have
 * to make sure to clean up this returned edge to avoid memory leak */
optional<shared_ptr<Transition> > ITree::findSendOfThisWait(const Transition& wait) const {
    auto & env = wait.getEnvelope();
    auto & state = GetCurrNode()->getState();
    optional<shared_ptr<Transition> > result;
    if (env.isWaitOrTestType()) {
        for (auto send : state.getRequestedProcs(wait)) {
            if (send->getEnvelope().isSendType()) {
                result.reset(send);
                return result;
            }
        }
    }
    return result;
}

/* going through all the traces, return the max number of MPI calls
 * Side effects: None */
size_t ITree::getMaxTlistSize() {
    return GetCurrNode()->getState().getMaxSize();
}

/* Return true if there exists a path from src to destination
 * We do not consider the wait node's since the sends are buffered
 * Side effects: visited array is used but not cleaned up.
 * The expectation is that whichever method that uses visited will
 * have to initialized it.
 * We're using BFS here, because DFS would result in a very deep
 * recursion.
 */
bool ITree::FindNonSendWaitPath(shared_ptr<Transition> src, shared_ptr<Transition> dest) {
    if (src == dest)
        return true;
    if (src->pid == dest->pid && src->index > dest->index)
        return false;

    std::queue<shared_ptr<Transition> > worklist;
    std::set<shared_ptr<Transition> > visited;

    /* doing a BFS search */
    worklist.push(src);

    while (worklist.size() > 0) {
        auto current = worklist.front();
        worklist.pop();
        if (current == dest) {
            return true;
        }
        visited.insert(current);
        /* now add all the intraCB edges of current into the worklist */
        for (auto intra : current->getIntraCB()) {
            if (auto send = findSendOfThisWait(*intra)) {
                visited.insert(intra);
            } else if (visited.find(intra) == visited.end()) {
                worklist.push(intra);
                visited.insert(intra);
            }
        }
        /* now add all the interCB edges of current into the worklist */
        for (auto inter : current->getInterCB()) {
            if (auto send = findSendOfThisWait(*inter)) {
                visited.insert(inter);
            } else if (visited.find(inter) == visited.end()) {
                worklist.push(inter);
                visited.insert(inter);
            }
        }
    }
    return false;

}

void ITree::FindCoEnabledSends () {
    /* keep visited CB edges here */
    std::set<shared_ptr<Transition> > visited;

    // all but the last one
    for (auto n : _slist) {
        /* Only care about nodes with wild card receive */
        if (!n->isWildcardNode()) continue;

        auto send = n->ample_set.front().front();
        n->wildcard.addSendsMatch(&send);
        std::map <CB, std::list <CB> >::iterator csi = aux_coenabled_sends.find(n->wildcard);
        if(csi != aux_coenabled_sends.end())
            csi->second.remove(send);

        auto renv = n->wildcard.getEnvelope();
        for (int i = 0 ; i < n->getNumProcs(); i++) {
            if (i == n->wildcard.pid) {
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
                            FindNonSendWaitPath(n->wildcard, c)) {
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
}

void ITree::ClearInterCB () {
    Node *n = GetCurrNode ();

    for (int i = 0 ; i < n->NumProcs (); i++) {
        for (unsigned int j = 0;  j < n->_tlist[i]->_tlist.size(); j++) {
            vector<CB>::iterator iter;
            n->_tlist[i]->_tlist[j].mod_inter_cb().clear();
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
                    (!n->isWildcardNode() ||
                    (n->isWildcardNode() && aux_coenabled_sends[n->wildcard].empty()))))) {

                delete last_node;
                return true;
        }

        /* if ample_set is empty, delete that node */
        if(n->isWildcardNode()) {
            matched_sends[n->wildcard].clear();
        }
        if (depth != oldDepth) {
            delete *(_slist.end()-1);
        }
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
