#ifndef _SEND_HPP
#define _SEND_HPP

#include "Receive.hpp"

struct Send {
    int count;

    int datatype;

    int dest;

    int tag;

    int comm;

    Send();

    Send(const Send &s);

    Send(int count, int datatype, int dest, int tag, int comm);

    bool canSend(const Receive & recv) const;

    bool operator== (const Send &s) const;
};

#endif
