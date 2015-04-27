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

#ifndef _CB_HPP
#define _CB_HPP
/*
#include "Envelope.hpp"
#include <vector>
#include <cassert>
*/
/*
 * This implements a single transition
 */
struct CB {
public:
    /** The issuer of the MPI function */
    int pid;
    /** The index of the MPI function */
    int index;

    CB() : pid(-1), index(-1) {}

    CB(int p, int i) : pid(p), index(i) {}

    bool operator== (const CB &c) {
        return pid == c.pid && index == c.index;
    }
    bool operator!= (const CB &c) {
        return !(*this == c);
    }
    // XXX: required by Node::createAllMatchingSends
    friend bool operator< (const CB &a, const CB &b) {
        return a.pid < b.pid || (a.pid == b.pid && a.index < b.index);
    }
};

#endif
