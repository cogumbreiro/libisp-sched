#include <vector>
#include <map>
#include <set>
#include <utility>

#include "name2id.hpp"
#include "Envelope.hpp"

using std::set;
using std::vector;
using std::map;

struct Call {
    int pid;
    int index;
    Envelope envelope;

    Call(int p, int i, Envelope e) : pid(p), index(i), envelope(e) {}
    Call(const Call & c) : pid(c.pid), index(c.index), envelope(c.envelope) {}
    Call() : pid(0), index(0) {}

    bool completesBefore(Call const&) const;

    friend bool operator< (const Call &a, const Call &b) {
        return a.pid < b.pid || (a.pid == b.pid && a.index < b.index);
    }

    bool operator== (const Call &c) const {
        return pid == c.pid && index == c.index;
    }

    bool operator!= (const Call &c) const {
        return !(*this == c);
    }

    /**
     * pre-condition: all members of `call` must have the same `pid` as this
     * instance.
     */
    bool hasAncestors(const set<Call> & call) const;
};

using Process = set<Call>;
