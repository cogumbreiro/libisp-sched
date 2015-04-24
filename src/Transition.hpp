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

class Transition_internal {
public:
    bool is_curr_matching_set();

    std::vector<int> ancestors;
    std::unique_ptr<CB> curr_matching;
};

class Transition {
public:
    /*
    Transition();
    Transition(const Transition &t_);
    Transition(std::unique_ptr<Envelope> e);
    */
    //Transition &operator=(const Transition &t_);

    inline Envelope& GetEnvelope () const {return *envelope;}
    bool AddIntraCB(std::unique_ptr<CB> c);
    bool AddInterCB(std::unique_ptr<CB> c);
    void moveCurrMatching();
    void set_curr_matching(std::unique_ptr<CB> c);
    CB &get_curr_matching() const;
    std::vector<int> &get_ancestors() const;
    std::vector<int> &mod_ancestors();

    std::unique_ptr<Transition_internal> t;
private:
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
    inline bool IsNew(const CB &c) const {
        return !vec_contains(intra_cb, c)  && !vec_contains(inter_cb, c);
    }

};
  
#endif
