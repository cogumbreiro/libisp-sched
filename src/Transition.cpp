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

#include <signal.h>

bool Transition::AddIntraCB(std::unique_ptr<CB> c) {
    if(IsNew(*c)) {
        intra_cb.push_back(std::move(c));
        return true;
    }
    return false;
}

bool Transition::AddInterCB(std::unique_ptr<CB> c) {
    if (c->_pid == -1 && c->_index == -1) {
        inter_cb.push_back (std::move(c));
        return false;
    }
    if(IsNew(*c)) {
        inter_cb.push_back(std::move(c));
        return true;
    }
    return false;
}

void Transition::set_curr_matching(std::unique_ptr<CB> c) {
    t->curr_matching = std::move(c);
}

CB &Transition::get_curr_matching() const {
    return *t->curr_matching;
}


std::vector<int> &Transition::get_ancestors() const {
    return t->ancestors;
}

std::vector<int> &Transition::mod_ancestors() {
    return t->ancestors;
}

bool Transition_internal::is_curr_matching_set() {
    return *curr_matching != CB(-1,-1);
}

