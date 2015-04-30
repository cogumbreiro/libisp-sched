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

#include <boost/range/adaptor/indirected.hpp>
#include <boost/range/adaptor/reversed.hpp>

#include "EnabledTransitions.hpp"

using boost::adaptors::indirect;
using boost::adaptors::reversed;

bool EnabledTransitions::allAncestorsMatched(const Transition & func) const {
    bool is_wait_or_test_type = func.getEnvelope().isWaitorTestType();
    auto pid = func.pid;
    for (auto & curr : indirect(func.getAncestors())) {
        auto curr_env = curr.getEnvelope();
        auto curr_func = curr_env.func_id;
        if (!matcher.isMatched(curr)) {
            // Wei-Fan Chiang: I don't know why I need to add this line. But, I need it........
            if (curr_func == PCONTROL) {
                continue;
            }
            if (is_wait_or_test_type && /* !Scheduler::_send_block*/
                    curr_func == ISEND) {
                continue;
            }
            return false;
        } else if (curr_func == WAIT ||
                   curr_func == TEST ||
                   curr_func == WAITALL ||
                   curr_func == TESTALL) {

            for (auto & anc : indirect(curr.getAncestors())) {
                if (!matcher.isMatched(anc) && curr_env.matchSend(anc.getEnvelope())) {
                    return false;
                }
            }
        }
    }
    return true;
}

bool EnabledTransitions::anyAncestorMatched (const Trace & trace, const Transition & func) const {
    bool any_match = false;

    bool is_wait_or_test_type = func.getEnvelope().isWaitorTestType();
    if (is_wait_or_test_type && func.getAncestors().empty()) {
        return true;
    }
    vector<shared_ptr<Transition> > filtered = func.getAncestors();
    for (auto req : trace.getRequestedProcs(func)) {
        if (matcher.isMatched(*req) ||
                (is_wait_or_test_type && /*!Scheduler::_send_block &&*/
                    req->getEnvelope().func_id == ISEND)) {
            any_match = true;
        }
        filtered.erase(std::remove(filtered.begin(), filtered.end(), req), filtered.end());
    }
    if (!any_match) {
        return false;
    }
    for (auto & curr : indirect(filtered)) {
        if(!matcher.isMatched(curr)) {
            return false;
        }
    }

    return true;
}

/* Note that the use of reverse iterator here is for performance reason
 * so that we can break off the loop when we hit the last_matched
 * transition - If we stop using reverse iterator - we need to stop using
 * reverse iterator in GetMatchingSends as well - otherwise we won't
 * be able to preserve the program order matching of receives/sends */
vector<shared_ptr<Transition> > EnabledTransitions::create() const {
    vector<shared_ptr<Transition> > result;

    for (Trace & trace : state) {
        int last = trace.size() - 1;
        for (auto func_ptr : trace.reverse()) {
            auto & func = *func_ptr;
            auto pid = func.pid;
            if (!matcher.isMatched(func)) {
                const auto func_name = func.getEnvelope().func_id;
                if ((func_name != WAITANY && func_name != TESTANY) &&
                        allAncestorsMatched(func)) {
                    result.push_back(func_ptr);
                } else if ((func_name == WAITANY || func_name == TESTANY) &&
                        anyAncestorMatched(trace, func)) {
                    result.push_back(func_ptr);
                }
            }
            last--;
            if (last <= matcher.findLastMatched(pid)) {
                break;
            }
        }
    }
    return result;
}
