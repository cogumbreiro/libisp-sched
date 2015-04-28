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
int Node::getTotalMpiCalls() const {
    int sum = 0;
    for (auto & trans : _tlist) {
      sum += trans->size();
    }
    return sum;
}*/

// XXX: move out of here
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

// XXX: move out of here
static list<CB> asHandles(const vector<MPIFunc> &funcs) {
    list<CB> result;
    for (auto func : funcs) {
        result.push_back(func.handle);
    }
    return result;
}

// XXX: move out of here
bool Node::addCollectiveAmple(const vector<MPIFunc> &funcs, int collective) {
    vector<MPIFunc> blist;
    vector<MPIFunc> flist;

    for (auto func : funcs) {
        if (func.envelope.func_id == collective) {
            blist.push_back(func);
        }
    }

    if (collective == FINALIZE) {
        if ((int)blist.size() == getNumProcs()) {
            ample_set.push_back(asHandles(blist));
            return true;
        }
        return false;
    }

    for (auto func1 : blist) {
        for (auto func2 : blist) {
            if (func2.envelope.comm == func1.envelope.comm) {
                flist.push_back(func2);
            }
        }
        if ((collective == BCAST || collective == GATHER ||
             collective == SCATTER || collective == SCATTERV ||
             collective == GATHERV || collective == REDUCE) && flist.size() > 0) {
            int root = (*flist.begin()).envelope.count;
            for (auto func2 : flist) {
                int other_root = func2.envelope.count;
                if (root != other_root) {
                    flist.clear();
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
                for (auto func2 : flist) {
                    colorcount[func2.envelope.comm_split_color] = 1;
                }

                /* Assign comm_id's to all of the new communicators. */
                for (auto kv : colorcount) {
                    kv.second = comm_id++;
                }

                /* Put the new IDs in the envelopes. */
                for (auto func2 : flist) {
                    auto e = func2.envelope;
                    e.comm_id = colorcount[e.comm_split_color];
                }
            } else {
                int id = comm_id++;
                for (auto func2 : flist) {
                    func2.envelope.comm_id = id;
                }
            }
        }

        if ((int)flist.size() == func1.envelope.nprocs) {
            break;
        } else {
            flist.clear ();
        }
    }

    if (flist.size() > 0) {
        ample_set.push_back(asHandles(flist));
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

// XXX: move out of here
bool Node::createAmpleSet() {
    if (ample_set.size() > 0) {
        return true;
    }

    auto enabled = enabledTransitions.create();

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
        auto last_handle = transitions.getLastHandle(pid);
        const auto & last = transitions.get(last_handle);
        if (last.getEnvelope().func_id == TEST ||
                last.getEnvelope().func_id == TESTALL ||
                last.getEnvelope().func_id == TESTANY ||
                last.getEnvelope().func_id == IPROBE) {
            test_list.push_back(last_handle);

            //Need to clean up the CB edge here
            //Each of this transition's ancestors will have an edge
            //to each of the transition's descendants
            for (int anc_id : last.getAncestors()) {
                for (CB desc : last.getIntraCB()) {
                    Transition & descendant = transitions.get(desc);
                    CB anc = CB(pid, anc_id);
                    Transition & ancestor = transitions.get(anc);
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
