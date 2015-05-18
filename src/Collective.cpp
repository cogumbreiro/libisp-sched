#include "Collective.hpp"

Collective::Collective() {}
Collective::Collective(const Collective &c) : data(c.data) {}

void Collective::set(Field f, int v) {
    data[f] = v;
}

optional<int> Collective::get(Field f) const {
    optional<int> result;
    auto iter = data.find(f);
    if (iter != data.end()) {
        result.reset(iter->second);
    }
    return result;
}

bool Collective::operator==(const Collective &c) {
    return data == c.data;
}
