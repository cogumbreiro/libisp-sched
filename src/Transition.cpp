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

bool Transition::addIntraCB(shared_ptr<Transition> t) {
    if(isNew(t)) {
        intra_cb.push_back(t);
        return true;
    }
    return false;
}

bool Transition::addInterCB(shared_ptr<Transition> t) {
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
