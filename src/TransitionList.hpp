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
 * File:        TransitionList.hpp
 * Description: Implements lists of transitions from an interleaving
 * Contact:     isp-dev@cs.utah.edu
 */

#ifndef _TRANSITIONLIST_HPP
#define _TRANSITIONLIST_HPP

#include <vector>
#include <list>
#include <algorithm>
#include <sstream>
#include <memory>
#include <boost/iterator/indirect_iterator.hpp>

#include "Envelope.hpp"
#include "Transition.hpp"

using std::unique_ptr;
using std::vector;
using boost::indirect_iterator;
using boost::make_indirect_iterator;

/*
 * This implements a TransitionList and updates "Intra-Completes-Before"
 */
class TransitionList {

public:
    inline int getId() const { return id; }
    inline unsigned int size() const { return tlist.size(); }
    inline Transition & get(int index) const { return *tlist[index]; }
    bool addTransition (unique_ptr<Transition> t);

    inline indirect_iterator<vector<unique_ptr<Transition> >::iterator> begin () {
        return make_indirect_iterator(tlist.begin());
    }

    inline indirect_iterator<vector<unique_ptr<Transition> >::iterator> end () {
        return make_indirect_iterator(tlist.end());
    }

    inline indirect_iterator<vector<unique_ptr<Transition> >::reverse_iterator> rbegin () {
        return make_indirect_iterator(tlist.rbegin());
    }

    inline indirect_iterator<vector<unique_ptr<Transition> >::reverse_iterator> rend () {
        return make_indirect_iterator(tlist.rend());
    }

private:
    std::vector<std::unique_ptr<Transition> > tlist;
    std::list<int> ulist;
    int id;
    inline bool intraCB (const Transition &f, const Transition &s) const {
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
        if (env_f.func_id == IRECV &&
                ((env_s.func_id == WAIT) ||
                 (env_s.func_id == TEST)) &&
                env_f.count == env_s.count) {
            return true;
        }

        if (env_f.func_id == ISEND &&
                ((env_s.func_id == WAIT) ||
                 (env_s.func_id == TEST)) &&
                env_f.count == env_s.count) {
            return true;
        }

        if (((env_s.func_id == WAITALL) ||
                (env_s.func_id == WAITANY) ||
                (env_s.func_id == TESTANY) ||
                (env_s.func_id == TESTALL) ) &&
                (env_f.func_id == IRECV ||
                 env_f.func_id == ISEND)) {

            for (int i = 0 ; i < env_s.count ; i++) {
                if (env_s.req_procs[i] == env_f.index) {
                    return true;
                }
            }
        }

        if (env_s.func_id == FINALIZE) {
            return true;
        }

        return false;
    }

};

#endif
