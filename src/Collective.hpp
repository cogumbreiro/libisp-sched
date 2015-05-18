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
    Sendtype,
    Recvcount,
    Recvtype,
    Color,
    Key
};

struct Collective {
    Collective();

    Collective(const Collective &c);

    void set(Field f, int v);

    optional<int> get(Field f) const;

    bool operator==(const Collective &c);

    Collective & operator=(const Collective & other);

    // Factories:

    static Collective barrier(int comm);

    static Collective bcast(int count, int datatype, int root, int comm);

    static Collective cart_create();

    static Collective comm_create(int comm);

    static Collective comm_dup(int comm);

    static Collective comm_split(int comm, int color, int key);

    static Collective comm_free(int comm);

    static Collective allreduce(int count, int datatype, int op, int comm);

    static Collective reduce(int count, int datatype, int op, int comm);

    static Collective gather(int sendcount, int sendtype, int recvcount, int recvtype, int root, int comm);

    static Collective scatter(int sendcount, int sendtype, int recvcount, int recvtype, int root, int comm);

    static Collective scatterv(int sendtype, int recvcount, int recvtype, int root, int comm);

    static Collective gatherv(int sendcount, int sendtype, int recvtype, int root, int comm);

    static Collective allgather(int sendcount, int sendtype, int recvcount, int recvtype, int comm);

    static Collective allgatherv(int sendcount, int sendtype, int recvtype, int comm);

    static Collective alltoall(int sendcount, int sendtype, int recvcount, int recvtype, int comm);

    static Collective alltoallv(int sendtype, int recvtype, int comm);

    static Collective scan(int count, int datatype, int op, int comm);

    static Collective reduce_scatter(int datatype, int op, int comm);

private:
    map<Field, int> data;
};

#endif
