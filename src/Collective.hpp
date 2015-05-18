#ifndef _COLLECTIVE_HPP
#define _COLLECTIVE_HPP

#include <map>
#include <boost/optional/optional.hpp>

using std::map;
using boost::optional;

enum class Field {
    Source,
    Destination,
    Datatype,
    Root,
    Communicator,
    Op,
    Count,
    Sendcount,
    Recvcount,
};

struct Collective {
    Collective();

    Collective(const Collective &c);

    void set(Field f, int v);

    optional<int> get(Field f) const;

    bool operator==(const Collective &c);

private:
    map<Field, int> data;
};

#endif
