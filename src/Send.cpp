#include "Send.hpp"

Send::Send() : count(0), datatype(0), dest(0), tag(0), comm(0) {}
Send::Send(const Send &s) : count(s.count), datatype(s.datatype), dest(s.dest),
        tag(s.tag), comm(s.comm) {}
Send::Send(int count, int datatype, int dest, int tag, int comm) : count(count), datatype(datatype), dest(dest),
        tag(tag), comm(comm) {}

bool Send::canSend(const Receive & recv) const {
    return count == recv.count
            && datatype == recv.datatype
            && recv.src.matches(dest)
            && recv.tag.matches(tag)
            && recv.comm == comm;
}

bool Send::operator==(const Send &s) const {
    return count == s.count
            && datatype == s.datatype
            && dest == s.dest
            && tag == s.tag
            && comm == s.comm;
}
