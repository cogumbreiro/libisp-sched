#define CATCH_CONFIG_MAIN
#include <catch.hpp>
#include "Call.hpp"
#include <map>
using std::multimap;
using std::pair;

bool Call::completesBefore(const Call & call) const {
    return pid == call.pid && index < call.index
            && envelope.completesBefore(call.envelope);
}

bool Call::hasAncestors(const set<Call> & calls) const {
    for (auto other : calls) {
        assert(pid == other.pid);
        if (index >= other.index) {
            return false;
        }
        if (other.completesBefore(*this)) {
            return true;
        }
    }
    return false;
}

bool Call::operator== (const Call &c) const {
    return pid == c.pid && index == c.index;
}

bool Call::operator!= (const Call &c) const {
    return !(*this == c);
}
