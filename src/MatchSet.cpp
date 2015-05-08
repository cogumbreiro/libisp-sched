#include "MatchSet.hpp"

void MatchSet::add(const Call &call) {
    data.push_back(call);
}

void MatchSet::concat(const MatchSet &rhs) {
    data.insert(data.end(), rhs.data.begin(), rhs.data.end());
}

vector<MatchSet> MatchSet::distribute(vector<MatchSet> many) const {
    vector<MatchSet> result;
    for (auto ms : many) {
        ms.concat(data);
        result.push_back(ms);
    }
    if (many.empty()) {
        result.push_back(data);
    }
    return result;
}

vector<Call> MatchSet::toVector() const {
    return data;
}
