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

#include "Transition.hpp"

bool Transition::addIntraCB(std::unique_ptr<CB> c) {
    if(isNew(*c)) {
        intra_cb.push_back(std::move(c));
        return true;
    }
    return false;
}

bool Transition::addInterCB(std::unique_ptr<CB> c) {
    if (c->_pid == -1 && c->_index == -1) {
        inter_cb.push_back (std::move(c));
        return false;
    }
    if(isNew(*c)) {
        inter_cb.push_back(std::move(c));
        return true;
    }
    return false;
}


