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
 * File:        Transition.hpp
 * Description: Implements handling of a single transition in an interleaving
 * Contact:     isp-dev@cs.utah.edu
 */

#ifndef _TRANSITION_HPP
#define _TRANSITION_HPP

#include <vector>
#include <cassert>
#include <memory>

#include "Envelope.hpp"

using std::vector;
using std::unique_ptr;
using std::weak_ptr;
using std::shared_ptr;

struct Transition {
    const int index;

    const int pid;

    bool types_match = true;

    Transition(int pid, int index, unique_ptr<Envelope> envelope) :
        pid(pid), index(index), envelope(std::move(envelope))
    {}

    inline Envelope& getEnvelope () const {return *envelope;}

    bool addIntraCB(shared_ptr<Transition> t) {
        if(isNew(t)) {
            intra_cb.push_back(t);
            return true;
        }
        return false;
    }

    bool addInterCB(shared_ptr<Transition> t) {
        /* XXX: Tiago: why is this needed?
        if (t.pid == -1 && t.index == -1) {
            inter_cb.push_back(t);
            return false;
        }*/
        if(isNew(t)) {
            inter_cb.push_back(t);
            return true;
        }
        return false;
    }
    inline void setCurrentMatching(shared_ptr<Transition> c) { curr_matching = c; }

    inline const shared_ptr<Transition> getCurrentMatching() const {
        return curr_matching.lock();
    }

    inline const vector<shared_ptr<Transition> > getAncestors() const { return share(ancestors); }

    inline void addAncestor(shared_ptr<Transition> ancestor) { ancestors.push_back(ancestor); }

    inline vector<shared_ptr<Transition> > getInterCB() const { return share(inter_cb); }

    inline vector<shared_ptr<Transition> > getIntraCB() const { return share(intra_cb); }

    inline bool canSend(Transition & recv) const {
        auto & recv_env = *recv.envelope;
        return pid == recv_env.src && envelope->canSend(recv_env);
    }

    inline void setMatched() { is_matched = true; }

    inline bool isMatched() const { return is_matched; }

    inline bool isIssued() const {
        return is_issued;
    }

private:
    vector<weak_ptr<Transition> > ancestors;
    weak_ptr<Transition> curr_matching;
    unique_ptr<Envelope> envelope;
    vector<weak_ptr<Transition> > inter_cb;
    vector<weak_ptr<Transition> > intra_cb;
    bool is_matched = false;
    bool is_issued = false;
    /**
     * Tests if an element is in the vector.
     */
    inline static bool contains(vector<weak_ptr<Transition> > vec, const Transition & elem) {
        for (auto ref : vec) {
            if (auto other = ref.lock()) {
                if (elem.handleEquals(*other)) {
                    return true;
                }
            }
        }
        return false;
    }

    static vector<shared_ptr<Transition> > share(const vector<weak_ptr<Transition> > & src) {
        vector<shared_ptr<Transition> > temp;
        temp.resize(src.size());
        for (auto weak : src) {
            if (auto val = weak.lock()) {
                temp.push_back(val);
            }
        }
        return temp;
    }

    inline bool handleEquals(const Transition & other) const {
        return pid == other.pid && index == other.index;
    }

    /**
     * Tests whether the transition contains the given cb.
     */
    inline bool isNew(shared_ptr<Transition> other) const {
        return !contains(intra_cb, *other)  && !contains(inter_cb, *other);
    }

};

#endif
