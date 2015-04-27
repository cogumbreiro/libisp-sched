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
 * File:        TransitionList.cpp
 * Description: Implements lists of transitions from an interleaving
 * Contact:     isp-dev@cs.utah.edu
 */

#include <iostream>
#include <sstream>
#include <boost/range/adaptor/reversed.hpp>
#include <boost/range/adaptor/indirected.hpp>

#include "TransitionList.hpp"

using boost::adaptors::reverse;
using boost::adaptors::indirect;

bool TransitionList::addTransition(std::unique_ptr<Transition> t) {
    auto & env_t = t->getEnvelope();
    const int index = env_t.index;
    // check if already in the list
    if (index <= size() - 1) {
        // return true only when the envelopes match
        return tlist[index]->getEnvelope() == env_t;
    }
    // otherwise append to the list
    tlist.push_back(std::move(t));
    auto & trans = tlist.back();
    const CB last_op(id, size()-1);

    bool blocking_flag = false;

    if (env_t.func_id == ISEND || env_t.func_id == IRECV) {
        ulist.push_back (index);
    } else if (env_t.func_id == WAIT || env_t.func_id == WAITALL ||
               env_t.func_id == TEST || env_t.func_id == TESTALL) {
        for (auto & req : env_t.req_procs) {
            if (tlist[req]->addIntraCB(last_op)) {
                t->addAncestor(req);
            }
            ulist.remove(req);
        }
    }

    int i = size() - 2;
    for (auto & curr : reverse(indirect(tlist))) {
        if (intraCB(curr, *t)) {
            if (blocking_flag) {
                if (env_t.func_id != SEND &&
                        (ulist.size() == 0 || index < ulist.front())) {
                    return true;
                }
                //if a blocking call occured in the past
                //the only calls that can slip through it are
                //Isend and Irecv
                auto env_f = curr.getEnvelope();
                if (env_f.func_id == IRECV) {
                    if (curr.addIntraCB(last_op)) {
                        t->addAncestor(i);
                    }

                    //terminate if this satisfies the Irecv intraCB rule
                    if (env_t.matchRecv(env_f)) {
                        return true;
                    }
                } else if (env_f.func_id == ISEND) {
                    if (curr.addIntraCB(last_op)) {
                        t->addAncestor(i);
                    }

                    //terminate if this satisfies the Isend intraCB rule
                    if (env_t.matchSend(env_f)) {
                        return true;
                    }
                }
            } else {
                if (curr.addIntraCB(last_op)) {
                    t->addAncestor(i);
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
