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
class CB {
public:
    int _pid;
    int _index;
    CB () { }

    CB (int i, int ind) : _pid(i), _index(ind) {
    }
    CB(const CB& c) : _pid(c._pid), _index(c._index){}
    bool operator== (const CB &c) {
        return _pid == c._pid && _index == c._index;
    }
    bool operator!= (const CB &c) {
        return !(*this == c);
    }
    friend bool operator< (const CB &a, const CB &b) {
        return a._pid < b._pid || (a._pid == b._pid && a._index < b._index);
    }
    /*
    CB& operator= (const CB &c) {
        this->_pid = c._pid;
        this->_index = c._index;
        return *this;
    }*/
    std::unique_ptr<CB> Copy() {
        std::unique_ptr<CB> ptr(new CB(*this));
        return std::move(ptr);
    }
};
  
#endif
