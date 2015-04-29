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
#include <boost/optional.hpp>
#include <boost/range/adaptor/indirected.hpp>

#include "Envelope.hpp"
#include "CB.hpp"

using std::vector;
using std::unique_ptr;
using std::weak_ptr;
using boost::optional;
using boost::adaptors::indirect;

struct Transition {
    const int index;

    const int pid;

    Transition(int pid, int index, unique_ptr<Envelope> envelope) :
        pid(pid), index(index), envelope(std::move(envelope))
    {}

    inline Envelope& getEnvelope () const {return *envelope;}

    bool addIntraCB(weak_ptr<Transition> t);

    bool addInterCB(weak_ptr<Transition> t);

    //inline void setCurrMatching(CB c) { curr_matching = c; }

    //inline const Transition& getCurrMatching() const { return curr_matching; }

    inline const vector<int> &getAncestors() const { return ancestors; }

    inline void addAncestor(const int ancestor) { ancestors.push_back(ancestor); }

    inline vector<weak_ptr<Transition> > getInterCB() const { return inter_cb; }

    inline vector<weak_ptr<Transition> > getIntraCB() const { return intra_cb; }

private:
    vector<int> ancestors;
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

    inline bool handleEquals(const Transition & other) const {
        return pid == other.pid && index == other.index;
    }

    /**
     * Tests whether the transition contains the given cb.
     */
    inline bool isNew(weak_ptr<Transition> ref) const {
        if (auto other = ref.lock()) {
            return !contains(intra_cb, *other)  && !contains(inter_cb, *other);
        }
        return false;
    }

};

#endif
