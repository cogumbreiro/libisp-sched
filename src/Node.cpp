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

vector<MPIFunc> Node::asMPIFunc(const vector<list<int> > &indices) const {
    vector<MPIFunc> result;
    for (int pid = 0; pid < getNumProcs(); pid++) {
        for (auto idx : indices[pid]) {
            CB op = CB(pid, idx);
            result.push_back(MPIFunc(op, getTransition(op).getEnvelope()));
        }
    }
    return result;
}

/* This is the exact same as the above, just with OpenMP disabled. */
/* Note that the use of reverse iterator here is for performance reason
 * so that we can break off the loop when we hit the last_matched
 * transition - If we stop using reverse iterator - we need to stop using
 * reverse iterator in GetMatchingSends as well - otherwise we won't
 * be able to preserve the program order matching of receives/sends */
vector<MPIFunc> Node::createEnabledTransitions() const {
    vector<list<int> > result;

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
    return asMPIFunc(result);
}

void Node::addWaitorTestAmple(const vector<MPIFunc> &funcs) {
    list <CB> blist;
    for (auto func : funcs) {
        if (func.envelope.isWaitorTestType()) {
            blist.push_back(func.handle);
        }
    }
    if (blist.size() > 0) {
        ample_set.push_back(blist);
    }
}

bool Node::addCollectiveAmple(const vector<MPIFunc> &funcs, int collective) {
    list <CB> blist;
    list <CB> flist;

    for (auto func : funcs) {
        if (func.envelope.func_id == collective) {
            blist.push_back(func.handle);
        }
    }

    if (collective == FINALIZE) {
        if ((int)blist.size () == getNumProcs()) {
            ample_set.push_back(blist);
            return true;
        }
        return false;
    }

    for (auto op1 : blist) {
        auto e1 = getTransition(op1).getEnvelope ();
        for (auto op2 : blist) {
            if (getTransition(op2).getEnvelope().comm == e1.comm) {
                flist.push_back(op2);
            }
        }
        if ((collective == BCAST || collective == GATHER ||
             collective == SCATTER || collective == SCATTERV ||
             collective == GATHERV || collective == REDUCE) && flist.size() > 0) {
            int root =
                getTransition(*flist.begin()).getEnvelope().count;
            for (auto op2 : flist) {
                int other_root = getTransition(op2).getEnvelope().count;
                if (root != other_root) {
                    flist.clear ();
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
                for (auto op2 : flist) {
                    colorcount[getTransition(op2).getEnvelope().comm_split_color] = 1;
                }

                /* Assign comm_id's to all of the new communicators. */
                for (auto kv : colorcount) {
                    kv.second = comm_id++;
                }

                /* Put the new IDs in the envelopes. */
                for (auto op2 : flist) {
                    auto e = getTransition(op2).getEnvelope();
                    e.comm_id = colorcount[e.comm_split_color];
                }
            } else {
                int id = comm_id++;
                for (auto op2 : flist) {
                    getTransition(op2).getEnvelope().comm_id = id;
                }
            }
        }

        if ((int)flist.size () == e1.nprocs) {
            break;
        } else {
            flist.clear ();
        }
    }

    if (flist.size() > 0) {
        ample_set.push_back(flist);
        return true;
    }
    return false;
}

// XXX: move this method to Matcher/InterleavingTree?
static optional<CB> getMatchingSend(const vector<MPIFunc> &funcs, MPIFunc recv) {
    optional<CB> result;
    /* The use of reverse_iterator here is necessary to preserve program-order
     * matching - This is based on the fact that GetEnabledTransistions also
     * uses a reverse iterator
     */
    for (auto snd : reverse(funcs)) {
        if (snd.canSend(recv)) {
            result.reset(snd.handle);
            return result;
        }
    }
    return result;
}

// XXX: move this method to Matcher/InterleavingTree?
bool Node::addNonWildcardReceive(const vector<MPIFunc> &funcs) {
    for (auto recv : funcs) {
        if (recv.envelope.isRecvType()) {
            if (recv.envelope.src != WILDCARD) {
                auto snd = getMatchingSend(funcs, recv);
                if(snd) {
                    list <CB> ml;
                    ml.push_back(snd.get());
                    ml.push_back(recv.handle);
                    ample_set.push_back(ml);
                    return true;
                }
            }
        }
    }
    return false;
}

// XXX: move this method to Matcher/InterleavingTree?
vector<list<CB> > Node::createAllMatchingSends(const vector<MPIFunc> &funcs, MPIFunc &recv) {
    vector<list<CB> > result;
    auto & sends = matcher.findMatchedSends(recv.handle);
    for (auto send : reverse(funcs)) {
        if (recv.canSend(send) && (sends.find(send.handle) == sends.end())) {
            list <CB> ml;
            ml.push_back(send.handle);
            ml.push_back(recv.handle);
            result.push_back(ml);
        }
    }
    return result;
}

// XXX: move this method to Matcher/InterleavingTree?
void Node::addAllSends(const vector<MPIFunc> &funcs) {
    bool first = true;
    for (auto recv : funcs) {
        if (recv.envelope.isRecvType() && recv.envelope.src == WILDCARD) {
            auto sends = createAllMatchingSends(funcs, recv);
            if (sends.size() > 0) {
                ample_set.insert(ample_set.end(), sends.begin(), sends.end());
                return;
            }
        }
    }
}

// XXX: move this method to Matcher/InterleavingTree?
void Node::addReceiveAmple(const vector<MPIFunc> &funcs) {
    if (addNonWildcardReceive(funcs)) {
        return;
    }
    addAllSends(funcs);
}

bool Node::createAmpleSet() {
    if (ample_set.size() > 0) {
        return true;
    }

    vector<MPIFunc> enabled = createEnabledTransitions();
    vector<int> collectives = {BARRIER, BCAST, SCATTER, GATHER, SCATTERV,
            GATHERV, ALLGATHER, ALLGATHERV, ALLTOALL, ALLTOALLV,
            SCAN, EXSCAN, REDUCE, REDUCE_SCATTER, ALLREDUCE, FINALIZE,
            CART_CREATE, COMM_CREATE, COMM_DUP, COMM_SPLIT, COMM_FREE};

    for (auto collective : collectives) {
        addCollectiveAmple(enabled, BARRIER);
        if (ample_set.size() > 0) {
            return true;
        }
    }

    addWaitorTestAmple(enabled);
    if (ample_set.size() > 0) {
        return true;
    }
    addReceiveAmple(enabled);
    if (ample_set.size() > 0) {
        return true;
    }

    /* Special case for Test & Iprobe calls
     * If no call can progress and
     * If there's a test call then we can match it and return false.
     * Also need to remove the CB edges */

    list<CB> test_list;
    for (int pid = 0; pid < getNumProcs(); pid++) {
        auto all = _tlist[pid];
        int last_id = all->size() - 1;
        const auto & last = all->get(last_id);
        if (last.getEnvelope().func_id == TEST ||
                last.getEnvelope().func_id == TESTALL ||
                last.getEnvelope().func_id == TESTANY ||
                last.getEnvelope().func_id == IPROBE) {
            test_list.push_back(CB(pid, last_id));

            //Need to clean up the CB edge here
            //Each of this transition's ancestors will have an edge
            //to each of the transition's descendants
            for (int anc_id : last.getAncestors()) {
                for (CB desc : last.getIntraCB()) {
                    Transition & descendant = getTransition(desc);
                    CB anc = CB(pid, anc_id);
                    Transition & ancestor = getTransition(anc);
                    descendant.addAncestor(anc_id);
                    ancestor.addIntraCB(desc);
                }
            }
        }
    }
    if (test_list.size() > 0) {
        ample_set.push_back(test_list);
        return true;
    }

    return false;
}
