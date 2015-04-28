#include "EnabledTransitions.hpp"

bool EnabledTransitions::allAncestorsMatched(const MPIFunc func, const vector <int> &indices) const {
    bool is_wait_or_test_type = func.envelope.isWaitorTestType();
    auto pid = func.handle.pid;
    for (auto curr : indices) {
        auto curr_handle = CB(pid, curr);
        auto & curr_trans = transitions.get(curr_handle);
        auto curr_env = curr_trans.getEnvelope();
        auto curr_func = curr_env.func_id;
        if (!matcher.isMatched(curr_handle)) {
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

            for (auto anc_id : curr_trans.getAncestors()) {
                auto anc = CB(pid, anc_id);
                auto & anc_env = transitions.getEnvelope(anc);
                if (!matcher.isMatched(anc) && curr_env.matchSend(anc_env)) {
                    return false;
                }
            }
        }
    }
    return true;
}

bool EnabledTransitions::anyAncestorMatched (const MPIFunc func, const vector <int> &indices) const {
    bool any_match = false;

    bool is_wait_or_test_type = func.envelope.isWaitorTestType();
    if (is_wait_or_test_type && indices.size() == 0) {
        return true;
    }
    vector<int> filtered = indices;
    for (auto idx : func.envelope.req_procs) {
        auto req = CB(func.handle.pid, idx);
        if (matcher.isMatched(req) ||
                (is_wait_or_test_type && /*!Scheduler::_send_block &&*/
                    transitions.getEnvelope(req).func_id == ISEND)) {
            any_match = true;
        }
        filtered.erase(std::remove(filtered.begin(), filtered.end(), idx), filtered.end());
    }
    if (!any_match) {
        return false;
    }
    int pid = func.handle.pid;
    for (auto id : filtered) {
        auto op = CB(pid, id);
        if(!matcher.isMatched(op)) {
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
vector<MPIFunc> EnabledTransitions::create() const {
    vector<MPIFunc> result;

    for (auto funcs : transitions.generateMPIFuncs()) {
        int last = funcs.size() - 1;
        for (auto func : reverse(funcs)) {
            auto pid = func.handle.pid;
            if (!matcher.isMatched(func.handle)) {
                vector<int> ancestors(transitions.get(func.handle).getAncestors());
                const auto func_name = func.envelope.func_id;
                if ((func_name != WAITANY && func_name != TESTANY) &&
                        allAncestorsMatched(func, ancestors)) {
                    result.push_back(func);
                } else if ((func_name == WAITANY || func_name == TESTANY) &&
                        anyAncestorMatched(func, ancestors)) {
                    result.push_back(func);
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
