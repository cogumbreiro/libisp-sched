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

#include <iostream>
#include <sstream>
#include <memory>
#include <boost/range/adaptor/reversed.hpp>
#include <boost/range/adaptor/indirected.hpp>

#include "Trace.hpp"

using boost::adaptors::reverse;
using boost::adaptors::indirect;
using std::make_shared;
using std::move;

bool Trace::intraCB (const Transition &f, const Transition &s) const {
    const auto &env_f = f.getEnvelope();
    const auto &env_s = s.getEnvelope();

    /*
     * Find Intra-Completes-Before :
     * 1) Blocking rule
     */
    if (env_f.isBlockingType ()) {
        return true;
    }

    /*
     * 2) Send order rule
     */

    if (env_f.isSendType () &&
        env_s.isSendType () &&
        env_f.dest == env_s.dest &&
        env_f.comm == env_s.comm &&
        env_f.stag == env_s.stag) {
        return true;
    }
    /*
     * 3) Recv order rule
     */
    if (env_f.isRecvType () &&
        env_s.isRecvType () &&
        (env_f.src == env_s.src ||
         env_f.src == WILDCARD) &&
        env_f.comm == env_s.comm &&
        (env_f.rtag == env_s.rtag ||
         env_f.rtag == WILDCARD)) {
        return true;
    }

    /*
     * 4) iRecv -> Wait order rule
     */
    if (env_f.func_id == OpType::IRECV &&
            ((env_s.func_id == OpType::WAIT) ||
             (env_s.func_id == OpType::TEST)) &&
            env_f.count == env_s.count) {
        return true;
    }

    if (env_f.func_id == OpType::ISEND &&
            ((env_s.func_id == OpType::WAIT) ||
             (env_s.func_id == OpType::TEST)) &&
            env_f.count == env_s.count) {
        return true;
    }

    if (((env_s.func_id == OpType::WAITALL) ||
            (env_s.func_id == OpType::WAITANY) ||
            (env_s.func_id == OpType::TESTANY) ||
            (env_s.func_id == OpType::TESTALL) ) &&
            (env_f.func_id == OpType::IRECV ||
             env_f.func_id == OpType::ISEND)) {

        for (int i = 0 ; i < env_s.count ; i++) {
            if (env_s.req_procs[i] == env_f.index) {
                return true;
            }
        }
    }

    if (env_s.func_id == OpType::FINALIZE) {
        return true;
    }

    return false;
}

bool Trace::add(std::unique_ptr<Envelope> env) {
    const int index = env->index;
    // check if already in the list
    if (index <= size() - 1) {
        // return true only when the envelopes match
        return tlist[index]->getEnvelope() == *env;
    }
    // otherwise append to the list
    auto last_op = make_shared<Transition>(pid, size() - 1, move(env));
    tlist.push_back(last_op);
    auto env_t = last_op->getEnvelope();

    bool blocking_flag = false;

    if (env_t.func_id == OpType::ISEND || env_t.func_id == OpType::IRECV) {
        ulist.push_back(last_op->index);
    } else if (env_t.func_id == OpType::WAIT || env_t.func_id == OpType::WAITALL ||
               env_t.func_id == OpType::TEST || env_t.func_id == OpType::TESTALL) {
        for (auto & req : env_t.req_procs) {
            auto curr = tlist[req];
            if (curr->addIntraCB(last_op)) {
                last_op->addAncestor(curr);
            }
            ulist.remove(curr->index);
        }
    }

    int i = size() - 2;
    for (auto & curr : indirect(reverse())) {
        if (intraCB(curr, *last_op)) {
            if (blocking_flag) {
                if (env_t.func_id != OpType::SEND &&
                        (ulist.size() == 0 || index < ulist.front())) {
                    return true;
                }
                //if a blocking call occured in the past
                //the only calls that can slip through it are
                //Isend and Irecv
                auto env_f = curr.getEnvelope();
                if (env_f.func_id == OpType::IRECV) {
                    if (curr.addIntraCB(last_op)) {
                        last_op->addAncestor(tlist[i]);
                    }

                    //terminate if this satisfies the Irecv intraCB rule
                    if (env_t.matchRecv(env_f)) {
                        return true;
                    }
                } else if (env_f.func_id == OpType::ISEND) {
                    if (curr.addIntraCB(last_op)) {
                        last_op->addAncestor(tlist[i]);
                    }

                    //terminate if this satisfies the Isend intraCB rule
                    if (env_t.matchSend(env_f)) {
                        return true;
                    }
                }
            } else {
                if (curr.addIntraCB(last_op)) {
                    last_op->addAncestor(tlist[i]);
                }

                /* avo 06/11/08 - trying not to add redundant edges */
                //a blocking call occured earlier
                //to avoid unnecessary CB edges
                if (curr.getEnvelope().isBlockingType()) {
                    blocking_flag = true;
                }
            }
        }
        i--;
    }
    return true;
}
