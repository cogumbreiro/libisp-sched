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

#ifndef _AMPLE_SET_HPP
#define _AMPLE_SET_HPP

//#define CONSOLE_OUTPUT_THRESHOLD 200

#include <vector>
#include <set>
#include <iterator>
#include <memory>
#include <boost/optional.hpp>

#include "Trace.hpp"
#include "Matcher.hpp"
#include "State.hpp"

using std::vector;
using std::shared_ptr;
//using boost::optional;

struct AmpleSet {

public:
    AmpleSet(State &s, const Matcher & m, const vector<shared_ptr<Transition>> & f):
        matcher(m),
        state(s),
        funcs(f)
    {}
    vector<vector<shared_ptr<Transition> > > create();
private:
    vector<vector<shared_ptr<Transition> > > ample_set;

    const Matcher & matcher;

    State & state;

    const vector<shared_ptr<Transition>> & funcs;

    bool genCollectiveAmple(int collective);

    bool genWaitorTestAmple();

    bool genReceiveAmple();

    bool genTestIProbe();

    bool genNonWildcardReceive();

    vector<vector<shared_ptr<Transition> > > createAllMatchingSends(shared_ptr<Transition>);

    bool genAllSends();
};

inline vector<vector<shared_ptr<Transition> > > createAmpleSet(State& s, const Matcher & m,
        const vector<shared_ptr<Transition> > & f) {
    return AmpleSet(s, m, f).create();
}

#endif
