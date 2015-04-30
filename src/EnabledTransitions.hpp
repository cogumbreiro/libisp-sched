#ifndef _ENABLED_TRANSITIONS_HPP
#define _ENABLED_TRANSITIONS_HPP

#include "Matcher.hpp"
#include "State.hpp"

struct EnabledTransitions {
    EnabledTransitions(const Matcher & m, const State & s) :
    matcher(m), state(s) {}
    vector<shared_ptr<Transition> > create() const;
private:
    const Matcher & matcher;

    const State & state;

    bool anyAncestorMatched(const Trace & trace, const Transition &) const;

    bool allAncestorsMatched(const Transition &) const;
};

#endif
