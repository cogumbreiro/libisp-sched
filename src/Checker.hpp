#ifndef _CHECKER_HPP
#define _CHECKER_HPP

#include <set>

#include "Call.hpp"

/* Function check should filter out any elements that are not enabled and not
 * matched. In the future it should issue an error if need be.
 */
set<Call> check(const set<Call> calls);

#endif
