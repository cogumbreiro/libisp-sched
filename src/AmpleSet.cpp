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
using std::vector;

bool AmpleSet::genWaitorTestAmple() {
    assert(ample_set.empty());
    vector<MPIFunc> blist;
    for (auto func : funcs) {
        if (func.envelope.isWaitorTestType()) {
            blist.push_back(func);
        }
    }
    if (blist.size() > 0) {
        ample_set.push_back(blist);
        return true;
    }
    return false;
}
/*
static vector<CB> asHandles(const vector<MPIFunc> &funcs) {
    vector<CB> result;
    for (auto func : funcs) {
        result.push_back(func.handle);
    }
    return result;
}*/

bool AmpleSet::genCollectiveAmple(int collective) {
    assert(ample_set.empty());

    vector<MPIFunc> blist;
    vector<MPIFunc> flist;

    for (auto func : funcs) {
        if (func.envelope.func_id == collective) {
            blist.push_back(func);
        }
    }

    if (collective == FINALIZE) {
        if ((int)blist.size() == transitions.num_procs) {
            ample_set.push_back(blist);
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
        ample_set.push_back(flist);
        return true;
    }
    return false;
}

static optional<MPIFunc> getMatchingSend(const vector<MPIFunc> & funcs, MPIFunc recv) {
    optional<MPIFunc> result;
    /* The use of reverse_iterator here is necessary to preserve program-order
     * matching - This is based on the fact that GetEnabledTransistions also
     * uses a reverse iterator
     */
    for (MPIFunc snd : reverse(funcs)) {
        if (snd.canSend(recv)) {
            result = snd;
            return result;
        }
    }
    return result;
}

bool AmpleSet::genNonWildcardReceive() {
    assert(ample_set.empty());
    for (auto recv : funcs) {
        if (recv.envelope.isRecvType()) {
            if (recv.envelope.src != WILDCARD) {
                auto snd = getMatchingSend(funcs, recv);
                if(snd) {
                    vector<MPIFunc> ml;
                    ml.push_back(*snd);
                    ml.push_back(recv);
                    ample_set.push_back(ml);
                    return true;
                }
            }
        }
    }
    return false;
}

vector<vector<MPIFunc> > AmpleSet::createAllMatchingSends(MPIFunc &recv) {
    vector<vector<MPIFunc> > result;
    auto & sends = matcher.findMatchedSends(recv.handle);
    for (auto send : reverse(funcs)) {
        if (recv.canSend(send) && (sends.find(send.handle) == sends.end())) {
            vector <MPIFunc> ml;
            ml.push_back(send);
            ml.push_back(recv);
            result.push_back(ml);
        }
    }
    return result;
}

// XXX: move this method to Matcher/InterleavingTree?
bool AmpleSet::genAllSends() {
    assert(ample_set.empty());
    bool first = true;
    for (auto recv : funcs) {
        if (recv.envelope.isRecvType() && recv.envelope.src == WILDCARD) {
            auto sends = createAllMatchingSends(recv);
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
    vector<MPIFunc> test_list;
    for (int pid = 0; pid < transitions.num_procs; pid++) {
        auto last_func = transitions.getLast(pid);
        const auto & last = transitions.get(last_func.handle);
        if (last.getEnvelope().func_id == TEST ||
                last.getEnvelope().func_id == TESTALL ||
                last.getEnvelope().func_id == TESTANY ||
                last.getEnvelope().func_id == IPROBE) {
            test_list.push_back(last_func);

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

// XXX: move out of here
vector<vector<MPIFunc> > AmpleSet::create() {
    vector<int> collectives = {BARRIER, BCAST, SCATTER, GATHER, SCATTERV,
            GATHERV, ALLGATHER, ALLGATHERV, ALLTOALL, ALLTOALLV,
            SCAN, EXSCAN, REDUCE, REDUCE_SCATTER, ALLREDUCE, FINALIZE,
            CART_CREATE, COMM_CREATE, COMM_DUP, COMM_SPLIT, COMM_FREE};

    for (auto collective : collectives) {
        if (genCollectiveAmple(BARRIER)) {
            return ample_set;
        }
    }
    genWaitorTestAmple() ||
    genReceiveAmple() ||
    genTestIProbe();
    return ample_set;
}
