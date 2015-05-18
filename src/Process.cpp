#include "Process.hpp"

Process::Process(int pid) : pid(pid), curr_handle(0) {}

Call Process::irecv(int count, int datatype, WInt src, WInt tag, int comm) {
    return create(OpType::IRECV, Receive(count, datatype, src, tag, comm));
}

Call Process::irecv(int src) {
    return irecv(WInt(src));
}

Call Process::irecv(WInt src) {
    return irecv(0, 0, src, WILDCARD, 0);
}

Call Process::recv(int count, int datatype, WInt src, WInt tag, int comm) {
    return create(OpType::RECV, Receive(count, datatype, src, tag, comm));
}

Call Process::send(int count, int datatype, int dest, int tag, int comm) {
    return create(OpType::SEND, Send(count, datatype, dest, tag, comm));
}

Call Process::ssend(int count, int datatype, int dest, int tag, int comm) {
    return create(OpType::SSEND, Send(count, datatype, dest, tag, comm));
}

Call Process::isend(int count, int datatype, int dest, int tag, int comm) {
    return create(OpType::ISEND, Send(count, datatype, dest, tag, comm));
}

Call Process::isend(int dest) {
    return isend(0, 0, dest, 0, 0);
}

Call Process::rsend(int count, int datatype, int dest, int tag, int comm) {
    return create(OpType::RSEND, Send(count, datatype, dest, tag, comm));
}

Call Process::wait(int request) {
    Call c = create(OpType::WAIT);
    c.wait.addRequest(request);
    return c;
}

Call Process::barrier(int comm) {
    return create(OpType::BARRIER, Collective::barrier(comm));
}

Call Process::bcast(int count, int datatype, int root, int comm) {
    return create(OpType::BCAST, Collective::bcast(count, datatype, root, comm));
}

Call Process::cart_create() {
    return create(OpType::CART_CREATE, Collective::cart_create());
}

Call Process::comm_create(int comm) {
    return create(OpType::COMM_CREATE, Collective::comm_create(comm));
}

Call Process::comm_dup(int comm) {
    return create(OpType::COMM_DUP, Collective::comm_dup(comm));
}

Call Process::comm_split(int comm, int color, int key) {
    return create(OpType::COMM_SPLIT, Collective::comm_split(comm, color, key));
}

Call Process::comm_free(int comm) {
    return create(OpType::COMM_FREE, Collective::comm_free(comm));
}

Call Process::allreduce(int count, int datatype, int op, int comm) {
    return create(OpType::ALLREDUCE, Collective::allreduce(count, datatype, op, comm));
}

Call Process::reduce(int count, int datatype, int op, int comm) {
    return create(OpType::REDUCE, Collective::reduce(count, datatype, op, comm));
}

Call Process::gather(int sendcount, int sendtype, int recvcount, int recvtype, int root, int comm) {
    return create(OpType::GATHER, Collective::gather(sendcount, sendtype, recvcount, recvtype, root, comm));
}

Call Process::scatter(int sendcount, int sendtype, int recvcount, int recvtype, int root, int comm) {
    return create(OpType::SCATTER, Collective::scatter(sendcount, sendtype, recvcount, recvtype, root, comm));
}

Call Process::scatterv(int sendtype, int recvcount, int recvtype, int root, int comm) {
    return create(OpType::SCATTERV, Collective::scatterv(sendtype, recvcount, recvtype, root, comm));
}

Call Process::gatherv(int sendcount, int sendtype, int recvtype, int root, int comm) {
    return create(OpType::GATHERV, Collective::gatherv(sendcount, sendtype, recvtype, root, comm));
}

Call Process::allgather(int sendcount, int sendtype, int recvcount, int recvtype, int comm) {
    return create(OpType::ALLGATHER, Collective::allgather(sendcount, sendtype, recvcount, recvtype, comm));
}

Call Process::allgatherv(int sendcount, int sendtype, int recvtype, int comm) {
    return create(OpType::ALLGATHERV, Collective::allgatherv(sendcount, sendtype, recvtype, comm));
}

Call Process::alltoall(int sendcount, int sendtype, int recvcount, int recvtype, int comm) {
    return create(OpType::ALLTOALL, Collective::alltoall(sendcount, sendtype, recvcount, recvtype, comm));
}

Call Process::alltoallv(int sendtype, int recvtype, int comm) {
    return create(OpType::ALLTOALLV, Collective::alltoallv(sendtype, recvtype, comm));
}

Call Process::scan(int count, int datatype, int op, int comm) {
    return create(OpType::SCAN, Collective::scan(count, datatype, op, comm));
}

Call Process::reduce_scatter(int datatype, int op, int comm) {
    return create(OpType::REDUCE_SCATTER, Collective::reduce_scatter(datatype, op, comm));
}

Call Process::finalize() {
    return create(OpType::FINALIZE);
}

Call Process::create(OpType call_type) {
    Call c;
    c.pid = pid;
    c.handle = curr_handle++;
    c.call_type = call_type;
    return c;
}

Call Process::create(OpType call_type, const Receive &recv) {
    Call c = create(call_type);
    c.recv = recv;
    return c;
}

Call Process::create(OpType call_type, const Send &send) {
    Call c = create(call_type);
    c.send = send;
    return c;
}

Call Process::create(OpType call_type, const Collective &col) {
    Call c = create(call_type);
    c.collective = col;
    return c;
}
