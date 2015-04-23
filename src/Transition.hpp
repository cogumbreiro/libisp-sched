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
#include <vector>
#include <cassert>

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
    CB& operator= (const CB &c) {
        this->_pid = c._pid;
        this->_index = c._index;
        return *this;
    }
};

class Transition_internal {
public:
    Transition_internal();
    ~Transition_internal();
    Transition_internal(Envelope * env);
    /*
      * Copy ctor and assignment operator
     */
    Transition_internal(Transition_internal &t);
    Transition_internal& operator=(Transition_internal &);

    inline Envelope *GetEnvelope () {return _env;}
    bool AddIntraCB_check(CB &c);
    void AddIntraCB_mutate(CB &c);
    bool AddInterCB_check(CB &c);
    void AddInterCB_mutate(CB &c);
    bool is_curr_matching_set();
    
#ifdef FIB
    bool AddCondIntraCB_check(CB &c);
    void AddCondIntraCB_mutate(CB &c);
#endif
    void ref_inc();
    void ref_dec();
    bool isshared();

    std::vector <CB> _inter_cb;
    std::vector <CB> _intra_cb;
#ifdef FIB
    std::vector <CB> _cond_intra_cb;
#endif

    std::vector<int> ancestors;
    CB curr_matching;
    unsigned int ref;
private:
    Envelope     *_env;
    /**
     * Tests if an element is in the vector.
     */
    inline static bool vec_contains(std::vector <CB> vec, CB &elem) {
        std::vector <CB>::iterator iter;
        std::vector <CB>::iterator iter_end = vec.end();
        
        for (iter = vec.begin (); iter != iter_end; iter++) {
            if (*iter == elem) {
                return true;
            }
        }
        return false;
    }

    inline bool Contains(CB &c) {
        return vec_contains(_intra_cb, c) || vec_contains(_inter_cb, c);
}
    
};

class Transition {
public:
    Transition();
    Transition(const Transition &t_, bool ref_t=false);
    Transition(Envelope *e, bool ref_e=false);
    
    Transition &operator=(const Transition &t_);

    void ref();
    void unref();
    void copy();
    Envelope *GetEnvelope() const;
    bool AddIntraCB(CB &c);
    void AddInterCB(CB &c);
    void moveCurrMatching();

    void set_curr_matching(const CB &c);
    CB &get_curr_matching() const;
    std::vector<int> &get_ancestors() const;
    std::vector<int> &mod_ancestors();
#ifdef FIB
    std::vector <CB> &get_inter_cb();
    std::vector <CB> &mod_inter_cb();
    std::vector <CB> &get_intra_cb();
    std::vector <CB> &get_cond_intra_cb();

    void AddCondIntraCB(CB &c);
#endif

    Transition_internal *t;
};
  
#endif
