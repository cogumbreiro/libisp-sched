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
#include <list>
#include <cassert>
#include <boost/optional.hpp>

#include "TransitionList.hpp"
#include "Matcher.hpp"
#include "TransitionMap.hpp"

using std::vector;
using boost::optional;

struct AmpleSet {

public:
    AmpleSet(TransitionMap & t, const Matcher & m, const vector<MPIFunc> & f):
        matcher(m),
        transitions(t),
        funcs(f)
    {}
    vector<vector<MPIFunc> > create();
private:
    vector<vector<MPIFunc> > ample_set;
    const Matcher & matcher;

    TransitionMap & transitions;

    const vector<MPIFunc> & funcs;

    bool genCollectiveAmple(int collective);

    bool genWaitorTestAmple();

    bool genReceiveAmple();

    bool genTestIProbe();

    bool genNonWildcardReceive();

    vector<vector<MPIFunc> > createAllMatchingSends(MPIFunc &recv);

    bool genAllSends();
};

inline vector<vector<MPIFunc> > createAmpleSet(TransitionMap & t, const Matcher & m,
        const vector<MPIFunc> & f) {
    return AmpleSet(t, m, f).create();
}

#endif
