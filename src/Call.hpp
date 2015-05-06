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

    friend bool operator< (const Call &a, const Call &b) {
        return a.pid < b.pid || (a.pid == b.pid && a.index < b.index);
    }

    bool operator== (const Call &c) {
        return pid == c.pid && index == c.index;
    }

    bool operator!= (const Call &c) {
        return !(*this == c);
    }

};

using Process = set<Call>;
