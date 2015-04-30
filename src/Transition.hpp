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

    Transition(int pid, int index, unique_ptr<Envelope> envelope) :
        pid(pid), index(index), envelope(std::move(envelope))
    {}

    inline Envelope& getEnvelope () const {return *envelope;}

    bool addIntraCB(shared_ptr<Transition> t);

    bool addInterCB(shared_ptr<Transition> t);

    //inline void setCurrMatching(CB c) { curr_matching = c; }

    //inline const Transition& getCurrMatching() const { return curr_matching; }

    inline const vector<shared_ptr<Transition> > getAncestors() const { return share(ancestors); }

    inline void addAncestor(shared_ptr<Transition> ancestor) { ancestors.push_back(ancestor); }

    inline vector<shared_ptr<Transition> > getInterCB() const { return share(inter_cb); }

    inline vector<shared_ptr<Transition> > getIntraCB() const { return share(intra_cb); }

private:
    vector<weak_ptr<Transition> > ancestors;
    //optional<Transition &> curr_matching;
    unique_ptr<Envelope> envelope;
    vector<weak_ptr<Transition> > inter_cb;
    vector<weak_ptr<Transition> > intra_cb;
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
