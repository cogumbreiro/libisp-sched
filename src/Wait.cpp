#include "Wait.hpp"

Wait::Wait(): requests() {}

Wait::Wait(const Wait &w) : requests(w.requests) {}

bool Wait::requested(int handle) const {
    return requests.find(handle) != requests.end();
}

void Wait::addRequest(int handle) {
    requests.insert(handle);
}

bool Wait::operator== (const Wait &w) const {
    return requests == w.requests;
}
