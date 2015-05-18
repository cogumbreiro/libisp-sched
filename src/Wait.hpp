#ifndef _WAIT_HPP
#define _WAIT_HPP

#include <set>

using std::set;

struct Wait {
    Wait();

    Wait(const Wait &w);

    bool requested(int handle) const;

    void addRequest(int handle);

    bool operator== (const Wait &w) const;

private:
    set<int> requests;
};

#endif
