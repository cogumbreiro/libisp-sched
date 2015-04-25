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

#include "TransitionList.hpp"

bool TransitionList::addTransition(std::unique_ptr<Transition> t) {
    int index = t->getEnvelope().index;
    int msize = (int) tlist.size();

    if (index <= msize-1) {
        if (t->getEnvelope() == tlist[index]->getEnvelope()) {
            tlist[index]->getEnvelope().issue_id = -1;
            //XXX: t->t = _tlist[index].t;
            return true;
        }
        return false;
    }

    tlist.push_back(std::move(t));
    auto & trans = tlist.back();
//    t = &_tlist.back();   // important! not a no-op
    int size = (int) tlist.size ();
    CB c(id, size-1);

    auto env_t = trans->getEnvelope();
    //const Envelope &env_f;
    bool blocking_flag = false;

    if (env_t.func_id == ISEND || env_t.func_id == IRECV) {
        ulist.push_back (index);
    } else if (env_t.func_id == WAIT || env_t.func_id == WAITALL ||
               env_t.func_id == TEST || env_t.func_id == TESTALL) {
        for (auto & req : env_t.req_procs) { 
            if (tlist[req]->addIntraCB(c.copy())) {
                t->addAncestor(req);
            }
            ulist.remove(req);
        }
        /*
        std::vector<int>::iterator it = env_t->req_procs.begin ();
        std::vector<int>::iterator it_end = env_t->req_procs.end ();
        for (; it != it_end; it++) {
            if (_tlist[*it].AddIntraCB(c)) {
                t->mod_ancestors().push_back(*it);
            }
            _ulist.remove (*it);
        }*/
    }

    auto iter = tlist.rbegin() + 1;
    auto iter_end = tlist.rend();

    int i = size - 2;
    for (; iter != iter_end; iter++) {
        auto & curr = **iter;
        if (intraCB(curr, *t)) {
            if (!blocking_flag) {
                if (curr.addIntraCB(c.copy())) {
                    t->addAncestor(i);
                }
                
                /* avo 06/11/08 - trying not to add redundant edges */
                auto env_f = curr.getEnvelope();
                
                //a blocking call occured earlier
                //to avoid unnecessary CB edges
                if (env_f.isBlockingType()) {
                    blocking_flag = true;
                }
            } else if (blocking_flag) {
                if (env_t.func_id != SEND && (ulist.size() == 0 || index < ulist.front())) {
                    return true;
                }
                //if a blocking call occured in the past
                //the only calls that can slip through it are
                //Isend and Irecv
                auto env_f = curr.getEnvelope();
                if (env_f.func_id == IRECV) {
                    if (curr.addIntraCB(c.copy())) {
                        t->addAncestor(i);
                    }

                    //terminate if this satisfies the Irecv intraCB rule
                    if (env_t.isRecvType() &&
                        env_f.src == env_t.src &&
                        env_f.comm == env_t.comm &&
                        env_f.rtag == env_t.rtag)
                        return true;
                } else if (env_f.func_id == ISEND) {
                    if (curr.addIntraCB(c.copy())) {
                        t->addAncestor(i);
                    }
                    
                    //terminate if this satisfies the Isend intraCB rule
                    if (env_t.isSendType() &&
                        env_f.dest == env_t.dest &&
                        env_f.comm == env_t.comm &&
                        env_f.stag == env_t.stag)
                        return true;
                }
            }
        }
        i--;
    }
    return true;
}
