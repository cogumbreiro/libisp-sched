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

Collective & Collective::operator=(const Collective & other) {
    data = other.data;
    return *this;
}


Collective Collective::barrier(int comm) {
    Collective c;
    c.set(Field::Communicator, comm);
    return c;
}

Collective Collective::bcast(int count, int datatype, int root, int comm) {
    Collective c;
    c.set(Field::Count, count);
    c.set(Field::Datatype, datatype);
    c.set(Field::Root, root);
    c.set(Field::Communicator, comm);
    return c;
}

Collective Collective::cart_create() {
    Collective c;
    return c;
}

Collective Collective::comm_create(int comm) {
    Collective c;
    c.set(Field::Communicator, comm);
    return c;
}

Collective Collective::comm_dup(int comm) {
    Collective c;
    c.set(Field::Communicator, comm);
    return c;
}

Collective Collective::comm_split(int comm, int color, int key) {
    Collective c;
    c.set(Field::Communicator, comm);
    c.set(Field::Color, color);
    c.set(Field::Key, key);
    return c;
}

Collective Collective::comm_free(int comm) {
    Collective c;
    c.set(Field::Communicator, comm);
    return c;
}

Collective Collective::allreduce(int count, int datatype, int op, int comm) {
    Collective c;
    c.set(Field::Count, count);
    c.set(Field::Datatype, datatype);
    c.set(Field::Op, op);
    c.set(Field::Communicator, comm);
    return c;
}

Collective Collective::reduce(int count, int datatype, int op, int comm) {
    return Collective::allreduce(count, datatype, op, comm);
}

Collective Collective::gather(int sendcount, int sendtype, int recvcount, int recvtype, int root, int comm) {
    Collective c;
    c.set(Field::Sendcount, sendcount);
    c.set(Field::Sendtype, sendtype);
    c.set(Field::Recvcount, recvcount);
    c.set(Field::Recvtype, recvtype);
    c.set(Field::Root, root);
    c.set(Field::Communicator, comm);
    return c;
}

Collective Collective::scatter(int sendcount, int sendtype, int recvcount, int recvtype, int root, int comm) {
    return Collective::gather(sendcount, sendtype, recvcount, recvtype, root, comm);
}

Collective Collective::scatterv(int sendtype, int recvcount, int recvtype, int root, int comm) {
    Collective c;
    c.set(Field::Sendtype, sendtype);
    c.set(Field::Recvcount, recvcount);
    c.set(Field::Recvtype, recvtype);
    c.set(Field::Root, root);
    c.set(Field::Communicator, comm);
    return c;
}

Collective Collective::gatherv(int sendcount, int sendtype, int recvtype, int root, int comm) {
    Collective c;
    c.set(Field::Sendcount, sendcount);
    c.set(Field::Sendtype, sendtype);
    c.set(Field::Recvtype, recvtype);
    c.set(Field::Root, root);
    c.set(Field::Communicator, comm);
    return c;
}

Collective Collective::allgather(int sendcount, int sendtype, int recvcount, int recvtype, int comm) {
    Collective c;
    c.set(Field::Sendcount, sendcount);
    c.set(Field::Sendtype, sendtype);
    c.set(Field::Recvcount, recvcount);
    c.set(Field::Recvtype, recvtype);
    c.set(Field::Communicator, comm);
    return c;
}

Collective Collective::allgatherv(int sendcount, int sendtype, int recvtype, int comm) {
    Collective c;
    c.set(Field::Sendcount, sendcount);
    c.set(Field::Sendtype, sendtype);
    c.set(Field::Recvtype, recvtype);
    c.set(Field::Communicator, comm);
    return c;
}

Collective Collective::alltoall(int sendcount, int sendtype, int recvcount, int recvtype, int comm) {
    return allgather(sendcount, sendtype, recvcount, recvtype, comm);
}

Collective Collective::alltoallv(int sendtype, int recvtype, int comm) {
    Collective c;
    c.set(Field::Sendtype, sendtype);
    c.set(Field::Recvtype, recvtype);
    c.set(Field::Communicator, comm);
    return c;
}

Collective Collective::scan(int count, int datatype, int op, int comm) {
    Collective c;
    c.set(Field::Count, count);
    c.set(Field::Datatype, datatype);
    c.set(Field::Op, op);
    c.set(Field::Communicator, comm);
    return c;
}

Collective Collective::reduce_scatter(int datatype, int op, int comm) {
    Collective c;
    c.set(Field::Datatype, datatype);
    c.set(Field::Op, op);
    c.set(Field::Communicator, comm);
    return c;
}
