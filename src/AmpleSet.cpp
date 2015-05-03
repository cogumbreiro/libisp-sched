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
#include <boost/range/adaptor/indirected.hpp>

#include "AmpleSet.hpp"

using boost::adaptors::reverse;
using boost::optional;
using std::vector;

bool AmpleSet::genWaitOrTestAmple() {
    assert(ample_set.empty());
    vector<shared_ptr<Transition>> blist;
    for (auto & func : funcs) {
        if (func->getEnvelope().isWaitOrTestType()) {
            blist.push_back(func);
        }
    }
    if (blist.size() > 0) {
        ample_set.push_back(blist);
        return true;
    }
    return false;
}

bool AmpleSet::genCollectiveAmple(OpType collective) {
    assert(ample_set.empty());

    vector<shared_ptr<Transition> > blist;
    vector<shared_ptr<Transition> > flist;

    for (auto func : funcs) {
        if (func->getEnvelope().func_id == collective) {
            blist.push_back(func);
        }
    }

    if (collective == OpType::FINALIZE) {
        if ((int)blist.size() == state.num_procs) {
            ample_set.push_back(blist);
            return true;
        }
        return false;
    }

    for (auto func1 : blist) {
        for (auto func2 : blist) {
            if (func2->getEnvelope().comm == func1->getEnvelope().comm) {
                flist.push_back(func2);
            }
        }
        if ((collective == OpType::BCAST || collective == OpType::GATHER ||
             collective == OpType::SCATTER || collective == OpType::SCATTERV ||
             collective == OpType::GATHERV || collective == OpType::REDUCE) && flist.size() > 0) {
            int root = (*flist.begin())->getEnvelope().count;
            for (auto func2 : flist) {
                int other_root = func2->getEnvelope().count;
                if (root != other_root) {
                    flist.clear();
                    return false;
                }
            }
        }

        if (collective == OpType::COMM_CREATE || collective == OpType::COMM_DUP ||
            collective == OpType::CART_CREATE || collective == OpType::COMM_SPLIT) {
            /* 0 = COMM_WORLD, 1 = COMM_SELF, 2 = COMM_NULL */
            static int comm_id = 3;
            if (collective == OpType::COMM_SPLIT) {
                std::map<int, int> colorcount;

                /* Mark which colors are being used in the map. */
                for (auto func2 : flist) {
                    colorcount[func2->getEnvelope().comm_split_color] = 1;
                }

                /* Assign comm_id's to all of the new communicators. */
                for (auto kv : colorcount) {
                    kv.second = comm_id++;
                }

                /* Put the new IDs in the envelopes. */
                for (auto func2 : flist) {
                    auto e = func2->getEnvelope();
                    e.comm_id = colorcount[e.comm_split_color];
                }
            } else {
                int id = comm_id++;
                for (auto func2 : flist) {
                    func2->getEnvelope().comm_id = id;
                }
            }
        }

        if ((int)flist.size() == func1->getEnvelope().nprocs) {
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

static optional<shared_ptr<Transition> > getMatchingSend(const vector<shared_ptr<Transition> > & funcs, const shared_ptr<Transition> recv_ptr) {
    optional<shared_ptr<Transition> > result;
    /* The use of reverse_iterator here is necessary to preserve program-order
     * matching - This is based on the fact that GetEnabledTransistions also
     * uses a reverse iterator
     */
    auto & recv = *recv_ptr;
    for (auto snd : reverse(funcs)) {
        if (snd->canSend(recv)) {
            result = snd;
            return result;
        }
    }
    return result;
}

bool AmpleSet::genNonWildcardReceive() {
    assert(ample_set.empty());
    for (auto recv_ptr : funcs) {
        auto & recv = *recv_ptr;
        if (recv.getEnvelope().isRecvType()) {
            if (recv.getEnvelope().src != WILDCARD) {
                auto snd = getMatchingSend(funcs, recv_ptr);
                if(snd) {
                    vector<shared_ptr<Transition> > ml;
                    ml.push_back(*snd);
                    ml.push_back(recv_ptr);
                    ample_set.push_back(ml);
                    return true;
                }
            }
        }
    }
    return false;
}

vector<vector<shared_ptr<Transition> > > AmpleSet::createAllMatchingSends(shared_ptr<Transition> recv_ptr) {
    vector<vector<shared_ptr<Transition> > > result;
    auto & recv = *recv_ptr;
    for (auto send_ptr : reverse(funcs)) {
        if (recv.canSend(*send_ptr) && recv.inMatchedSends(*send_ptr)) {
            vector<shared_ptr<Transition> > ml;
            ml.push_back(send_ptr);
            ml.push_back(recv_ptr);
            result.push_back(ml);
        }
    }
    return result;
}

bool AmpleSet::genAllSends() {
    assert(ample_set.empty());
    bool first = true;
    for (auto recv_ptr : funcs) {
        auto & recv_env = recv_ptr->getEnvelope();
        if (recv_env.isRecvType() && recv_env.src == WILDCARD) {
            auto sends = createAllMatchingSends(recv_ptr);
            if (sends.size() > 0) {
                ample_set = sends;
                return true;
            }
        }
    }
    return false;
}

bool AmpleSet::genReceiveAmple() {
    assert(ample_set.empty());
    return genNonWildcardReceive() || genAllSends();
}

bool AmpleSet::genTestIProbe() {
    assert(ample_set.empty());
    /* Special case for Test & Iprobe calls
     * If no call can progress and
     * If there's a test call then we can match it and return false.
     * Also need to remove the CB edges */
    vector<shared_ptr<Transition> > test_list;
    for (int pid = 0; pid < state.num_procs; pid++) {
        auto last_ptr = state.getLast(pid);
        auto & last = *last_ptr;
        //const auto & last = transitions.get(last_func.handle);
        if (last.getEnvelope().func_id == OpType::TEST ||
                last.getEnvelope().func_id == OpType::TESTALL ||
                last.getEnvelope().func_id == OpType::TESTANY ||
                last.getEnvelope().func_id == OpType::IPROBE) {
            test_list.push_back(last_ptr);

            //Need to clean up the CB edge here
            //Each of this transition's ancestors will have an edge
            //to each of the transition's descendants
            for (auto anc : last.getAncestors()) {
                for (auto desc : last.getIntraCB()) {
                    desc->addAncestor(anc);
                    anc->addIntraCB(desc);
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

// XXX: move out of here
vector<vector<shared_ptr<Transition> > > AmpleSet::create() {
    vector<OpType> collectives = {OpType::BARRIER, OpType::BCAST, OpType::SCATTER,
        OpType::GATHER, OpType::SCATTERV, OpType::GATHERV, OpType::ALLGATHER,
        OpType::ALLGATHERV, OpType::ALLTOALL, OpType::ALLTOALLV, OpType::SCAN,
        OpType::EXSCAN, OpType::REDUCE, OpType::REDUCE_SCATTER,
        OpType::ALLREDUCE, OpType::FINALIZE, OpType::CART_CREATE,
        OpType::COMM_CREATE, OpType::COMM_DUP, OpType::COMM_SPLIT,
        OpType::COMM_FREE};

    for (auto collective : collectives) {
        if (genCollectiveAmple(collective)) {
            return ample_set;
        }
    }
    genWaitOrTestAmple() ||
    genReceiveAmple() ||
    genTestIProbe();
    return ample_set;
}
