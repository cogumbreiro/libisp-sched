#include "MatchSet.hpp"

void MatchSet::add(const Call &call) {
    data.push_back(call);
}

void MatchSet::append(const MatchSet &other) {
    data.insert(data.end(), other.data.begin(), other.data.end());
}

void MatchSet::prepend(const MatchSet &other) {
    data.insert(data.begin(), other.data.begin(), other.data.end());
}

vector<Call> MatchSet::toVector() const {
    return data;
}

bool MatchSet::empty() const {
    return data.empty();
}
