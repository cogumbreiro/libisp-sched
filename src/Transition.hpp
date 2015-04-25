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

#include "Envelope.hpp"
#include "CB.hpp"
#include <vector>
#include <cassert>
#include <memory>

class Transition {
public:
    inline Envelope& getEnvelope () const {return *envelope;}
    bool addIntraCB(std::unique_ptr<CB> c);
    bool addInterCB(std::unique_ptr<CB> c);
    //void moveCurrMatching();

    void setCurrMatching(std::unique_ptr<CB> c) {
        curr_matching = std::move(c);
    }

    const CB & getCurrMatching() const {
        return *curr_matching;
    }

    inline const std::vector<int> &getAncestors() const {
        return ancestors;
    }

    void addAncestor(const int ancestor) {
        ancestors.push_back(ancestor);
    }
    //std::vector<int> &mod_ancestors();

private:
    std::vector<int> ancestors;
    std::unique_ptr<CB> curr_matching;
    std::unique_ptr<Envelope> envelope;
    std::vector <std::unique_ptr<CB> > inter_cb;
    std::vector <std::unique_ptr<CB> > intra_cb;
    /**
     * Tests if an element is in the vector.
     */
    inline static bool vec_contains(const std::vector <std::unique_ptr<CB> > &vec, const CB &elem) {
        for (auto & other : vec) {
            if (*other == elem) {
                return true;
            }
        }
        return false;
    }
    /**
     * Tests whether the transition contains the given cb.
     */
    inline bool isNew(const CB &c) const {
        return !vec_contains(intra_cb, c)  && !vec_contains(inter_cb, c);
    }

};
  
#endif
