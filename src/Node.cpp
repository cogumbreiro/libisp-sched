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

#include <sstream>
#include <map>
#include <queue>
#include <string.h>
#include <boost/range/adaptor/reversed.hpp>

#include "Node.hpp"

using boost::adaptors::reverse;
using std::list;
using std::vector;

/*
Node::Node (int num_procs) : has_child (false), _level(0),
        _num_procs(num_procs) {

    type = GENERAL_NODE;
#ifdef CONFIG_BOUNDED_MIXING
    expand = false;
#endif
    has_aux_coenabled_sends = false;
    for (int i = 0; i< num_procs ;i++) {
        _tlist.push_back(new TransitionList (i));
    }
}
*/
/*
int Node::getTotalMpiCalls() const {
    int sum = 0;
    for (auto & trans : _tlist) {
      sum += trans->size();
    }
    return sum;
}*/

vector<vector<shared_ptr<Transition> > > Node::buildAmpleSet() {
    auto enabled = enabledTransitions.create();
    return createAmpleSet(*state, matcher, enabled);
}
