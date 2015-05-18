#include "Receive.hpp"

Receive::Receive() : count(0), datatype(0), src(0), tag(0), comm(0) {}

Receive::Receive(const Receive &r) : count(r.count), datatype(r.datatype),
    src(r.src), tag(r.tag), comm(r.comm) {}

Receive::Receive(int count, int datatype, WInt src, WInt tag, int comm) :
    count(count), datatype(datatype), src(src), tag(tag), comm(comm) {}

bool Receive::matches(const Receive &rhs) const {
    return count == rhs.count
            && datatype == rhs.datatype
            && src.matches(rhs.src)
            && tag.matches(rhs.tag)
            && comm == rhs.comm;
}

bool Receive::operator== (const Receive &r) const {
    return count == r.count
            && datatype == r.datatype
            && src == r.src
            && tag == r.tag
            && comm == r.comm;
}
