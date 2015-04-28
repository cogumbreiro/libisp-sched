#ifndef _ENABLED_TRANSITIONS_FACTORY_HPP
#define _ENABLED_TRANSITIONS_FACTORY_HPP

#include "Matcher.hpp"
#include "TransitionMap.hpp"

struct EnabledTransitionsFactory {
    EnabledTransitionsFactory(const Matcher & m, const TransitionMap & t) :
    matcher(m), transitions(t) {}
    vector<MPIFunc> create() const;
private:
    const Matcher & matcher;

    const TransitionMap & transitions;

    bool anyAncestorMatched (const MPIFunc func, const vector <int> &indices) const;

    bool allAncestorsMatched(const MPIFunc func, const vector <int> &indices) const;
};

#endif
