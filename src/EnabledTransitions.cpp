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
    bool is_wait_or_test_type = func.getEnvelope().isWaitType()
            || func.getEnvelope().isTestType();
    auto pid = func.pid;
    for (auto & curr : indirect(func.getAncestors())) {
        auto curr_env = curr.getEnvelope();
        auto curr_func = curr_env.func_id;
        if (!curr.isMatched()) {
            // Wei-Fan Chiang: I don't know why I need to add this line. But, I need it........
            if (curr_func == OpType::PCONTROL) {
                continue;
            }
            if (is_wait_or_test_type && /* !Scheduler::_send_block*/
                    curr_func == OpType::ISEND) {
                continue;
            }
            return false;
        } else if (curr_func == OpType::WAIT ||
                   curr_func == OpType::TEST ||
                   curr_func == OpType::WAITALL ||
                   curr_func == OpType::TESTALL) {
            for (auto & anc : indirect(curr.getAncestors())) {
                if (!anc.isMatched() && curr_env.matchSend(anc.getEnvelope())) {
                    return false;
                }
            }
        }
    }
    return true;
}

bool EnabledTransitions::anyAncestorMatched (const Trace & trace, const Transition & func) const {
    bool any_match = false;

    bool is_wait_or_test_type = func.getEnvelope().isWaitType()
            || func.getEnvelope().isTestType();
    if (is_wait_or_test_type && func.getAncestors().empty()) {
        return true;
    }
    vector<shared_ptr<Transition> > filtered = func.getAncestors();
    for (auto req : trace.getRequestedProcs(func)) {
        if (req->isMatched() ||
                (is_wait_or_test_type && /*!Scheduler::_send_block &&*/
                    req->getEnvelope().func_id == OpType::ISEND)) {
            any_match = true;
        }
        filtered.erase(std::remove(filtered.begin(), filtered.end(), req), filtered.end());
    }
    if (!any_match) {
        return false;
    }
    for (auto & curr : indirect(filtered)) {
        if(!curr.isMatched()) {
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
            if (!func.isMatched()) {
                const auto func_name = func.getEnvelope().func_id;
                if ((func_name != OpType::WAITANY && func_name != OpType::TESTANY) &&
                        allAncestorsMatched(func)) {
                    result.push_back(func_ptr);
                } else if ((func_name == OpType::WAITANY || func_name == OpType::TESTANY) &&
                        anyAncestorMatched(trace, func)) {
                    result.push_back(func_ptr);
                }
            }
            last--;
            if (auto last_matched = trace.findLastMatched()) {
                if (last <= (*last_matched)->index) {
                    break;
                }
            }
        }
    }
    return result;
}
