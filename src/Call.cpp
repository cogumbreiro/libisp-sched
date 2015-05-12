#include "Call.hpp"

bool Call::completesBefore(const Call & call) const {
    return pid == call.pid && handle < call.handle
            && envelope.completesBefore(call.envelope);
}

bool Call::hasAncestors(const set<Call> & calls) const {
    for (auto other : calls) {
        assert(pid == other.pid);
        if (other.handle >= handle) {
            return false;
        }
        if (other.completesBefore(*this)) {
            return true;
        }
    }
    return false;
}

bool Call::operator== (const Call &c) const {
    return pid == c.pid && handle == c.handle;
}

bool Call::operator!= (const Call &c) const {
    return !(*this == c);
}
