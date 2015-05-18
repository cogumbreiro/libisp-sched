#include "WInt.hpp"

WInt::WInt() : wildcard(true), value(0)  {}
WInt::WInt(int v) : wildcard(false), value(v) {}
WInt::WInt(const WInt &c) : wildcard(c.wildcard), value(c.value) {}
bool WInt::operator== (const WInt &r) const {
    return wildcard == r.wildcard && value == r.value;
}

WInt & WInt::operator=(const WInt & other) {
    wildcard = other.wildcard;
    value = other.value;
    return *this;
}

bool WInt::isWildcard() const {
    return wildcard;
}
int WInt::get() const {
    return value;
}
bool WInt::matches(const WInt &other) const {
    return isWildcard() || *this == other;
}
