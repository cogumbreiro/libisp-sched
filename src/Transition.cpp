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

bool Transition::addIntraCB(const CB handle) {
    if(isNew(handle)) {
        intra_cb.push_back(handle);
        return true;
    }
    return false;
}

bool Transition::addInterCB(const CB handle) {
    if (handle.pid == -1 && handle.index == -1) {
        inter_cb.push_back(handle);
        return false;
    }
    if(isNew(handle)) {
        inter_cb.push_back(handle);
        return true;
    }
    return false;
}
