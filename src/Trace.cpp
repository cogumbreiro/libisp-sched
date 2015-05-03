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

bool Trace::add(std::unique_ptr<Envelope> env) {
    const int index = env->index;
    // check if already in the list
    if (index <= size() - 1) {
        // return true only when the envelopes match
        return trace[index]->getEnvelope() == *env;
    }
    // otherwise append to the list
    auto last_op = make_shared<Transition>(pid, size() - 1, move(env));
    trace.push_back(last_op);
    auto env_t = last_op->getEnvelope();

    bool blocking_flag = false;

    if (env_t.func_id == OpType::ISEND || env_t.func_id == OpType::IRECV) {
        ulist.push_back(last_op->index);
    } else if (env_t.func_id == OpType::WAIT || env_t.func_id == OpType::WAITALL ||
               env_t.func_id == OpType::TEST || env_t.func_id == OpType::TESTALL) {
        for (auto & req : env_t.req_procs) {
            auto curr = trace[req];
            if (curr->addIntraCB(last_op)) {
                last_op->addAncestor(curr);
            }
            ulist.remove(curr->index);
        }
    }

    int i = size() - 2;
    for (auto & curr : indirect(reverse())) {
        if (curr.getEnvelope().completesBefore(env_t)) {
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
                        last_op->addAncestor(trace[i]);
                    }

                    //terminate if this satisfies the Irecv intraCB rule
                    if (env_t.matchRecv(env_f)) {
                        return true;
                    }
                } else if (env_f.func_id == OpType::ISEND) {
                    if (curr.addIntraCB(last_op)) {
                        last_op->addAncestor(trace[i]);
                    }

                    //terminate if this satisfies the Isend intraCB rule
                    if (env_t.matchSend(env_f)) {
                        return true;
                    }
                }
            } else {
                if (curr.addIntraCB(last_op)) {
                    last_op->addAncestor(trace[i]);
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
