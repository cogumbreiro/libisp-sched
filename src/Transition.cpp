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
 * File:        Transition.cpp
 * Description: Implements handling of a single transition in an interleaving
 * Contact:     isp-dev@cs.utah.edu
 */

#ifdef CONFIG_DEBUG_SCHED
#define DR(code) if(Scheduler::_debug) { code }
#define DS(code) { code }
#else
#define DR(code)
#define DS(code)
#endif

#include "Transition.hpp"

#include <signal.h>

Transition::Transition() {
    DS( t = NULL; );
}

Transition::Transition(const Transition &t_, bool ref_t) {
    *this = t_;
    if(ref_t) t->ref_inc();
}

Transition::Transition(Envelope *e, bool ref_e) {
    t = new Transition_internal(e);
    if(ref_e) t->GetEnvelope()->ref_inc();
}

Transition &Transition::operator=(const Transition &t_) {
    t = t_.t;
    return *this;
}

void Transition::ref() {
    t->ref_inc();
}

void Transition::unref() {
    if(!t->isshared()) {
        delete t;
        DS( t = NULL; )
    } else t->ref_dec();
}

void Transition::copy() {
    if(t->isshared()) {
        t->ref_dec();
        t = new Transition_internal(*t);
    }
}

Envelope *Transition::GetEnvelope() const {
    return t->GetEnvelope();
}

bool Transition::AddIntraCB(CB &c) {
    if(t->AddIntraCB_check(c)) {
        copy();
        t->AddIntraCB_mutate(c);
        return true;
    }
    return false;
}

void Transition::AddInterCB(CB &c) {
    if(t->AddInterCB_check(c)) {
        copy();
        t->AddInterCB_mutate(c);
    }
}

void Transition::set_curr_matching(const CB &c) {
    copy();
    t->curr_matching = c;
}

CB &Transition::get_curr_matching() const {
    return t->curr_matching;
}


std::vector<int> &Transition::get_ancestors() const {
    return t->ancestors;
}

std::vector<int> &Transition::mod_ancestors() {
    copy();
    return t->ancestors;
}


#ifdef FIB
std::vector <CB> &Transition::get_inter_cb() {
    return t->_inter_cb;
}

std::vector <CB> &Transition::mod_inter_cb() {
    copy();
    return t->_inter_cb;
}

std::vector <CB> &Transition::get_intra_cb() {
    return t->_intra_cb;
}

std::vector <CB> &Transition::get_cond_intra_cb() {
    return t->_cond_intra_cb;
}
    
void Transition::AddCondIntraCB(CB &c) {
    if(t->AddCondIntraCB_check(c)) {
        copy();
        t->AddCondIntraCB_mutate(c);
    } 
}
#endif

Transition_internal::Transition_internal (Envelope * env) {
    _env = env;
    ref = 1;
    curr_matching = CB(-1,-1);
}

Transition_internal::Transition_internal (Transition_internal &t) {
    (*this) = t;
}

Transition_internal::~Transition_internal() {
    if(!_env->isshared()) {
        delete _env;
        DS( _env = NULL; )
    } else _env->ref_dec();
}

Transition_internal &Transition_internal::operator= (Transition_internal &t) {
    ref = 1;
    curr_matching = t.curr_matching;
    _env = t.GetEnvelope();
    _env->ref_inc();
    ancestors = t.ancestors;
    _intra_cb = t._intra_cb;
    _inter_cb = t._inter_cb;
#ifdef FIB
    _cond_intra_cb = t._cond_intra_cb;
#endif
    return *this;
}

bool Transition_internal::AddIntraCB_check(CB &c) {
    return !Contains(c);
}

void Transition_internal::AddIntraCB_mutate(CB &c) {
    _intra_cb.push_back(c);
}

bool Transition_internal::AddInterCB_check(CB &c) {
    std::vector <CB>::iterator iter;
    std::vector <CB>::iterator iter_end = _intra_cb.end();

    if (c._pid == -1 && c._index == -1) {
        _inter_cb.push_back (c);
        return false;
    }
    return !Contains(c);
}
void Transition_internal::AddInterCB_mutate(CB &c) {
    _inter_cb.push_back(c);
}

#ifdef FIB
bool Transition_internal::AddCondIntraCB_check(CB &c)
{
    return !vec_contains(_cond_intra_cb) && !vec_contains(_inter_cb);
}

void Transition_internal::AddCondIntraCB_mutate(CB &c) {
    _cond_intra_cb.push_back(c);
}
#endif

bool Transition_internal::is_curr_matching_set() {
    return curr_matching != CB(-1,-1);
}

void Transition_internal::ref_inc() {
    ref++;
}

void Transition_internal::ref_dec() {
    DS( assert(ref>1); )
    ref--;
}

bool Transition_internal::isshared() {
    DS( assert(ref>0); )
    return ref != 1;
}
