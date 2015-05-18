#ifndef _RECEIVE_HPP
#define _RECEIVE_HPP

#include "WInt.hpp"

struct Receive {
    int count;

    int datatype;

    WInt src;

    WInt tag;

    int comm;

    Receive();

    Receive(const Receive &r);

    Receive(int count, int datatype, WInt src, WInt tag, int comm);

    bool matches(const Receive &rhs) const;

    bool operator== (const Receive &r) const;
};

#endif
