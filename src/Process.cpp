#include "Process.hpp"

Process::Process(int pid) : pid(pid), curr_handle(0) {}

Call Process::irecv(int count, int datatype, WInt src, WInt tag, int comm) {
    return create(CallType::IRECV, Receive(count, datatype, src, tag, comm));
}

Call Process::recv(int count, int datatype, WInt src, WInt tag, int comm) {
    return create(CallType::RECV, Receive(count, datatype, src, tag, comm));
}

Call Process::send(int count, int datatype, int dest, int tag, int comm) {
    return create(CallType::SEND, Send(count, datatype, dest, tag, comm));
}

Call Process::ssend(int count, int datatype, int dest, int tag, int comm) {
    return create(CallType::SSEND, Send(count, datatype, dest, tag, comm));
}

Call Process::isend(int count, int datatype, int dest, int tag, int comm) {
    return create(CallType::ISEND, Send(count, datatype, dest, tag, comm));
}

Call Process::rsend(int count, int datatype, int dest, int tag, int comm) {
    return create(CallType::RSEND, Send(count, datatype, dest, tag, comm));
}

Call Process::wait(int request) {
    Call c = create(CallType::WAIT);
    c.wait.addRequest(request);
    return c;
}

Call Process::barrier(int comm) {
    return create(CallType::BARRIER, Collective::barrier(comm));
}

Call Process::bcast(int count, int datatype, int root, int comm) {
    return create(CallType::BCAST, Collective::bcast(count, datatype, root, comm));
}

Call Process::cart_create() {
    return create(CallType::CART_CREATE, Collective::cart_create());
}

Call Process::comm_create(int comm) {
    return create(CallType::COMM_CREATE, Collective::comm_create(comm));
}

Call Process::comm_dup(int comm) {
    return create(CallType::COMM_DUP, Collective::comm_dup(comm));
}

Call Process::comm_split(int comm) {
    return create(CallType::COMM_SPLIT, Collective::comm_split(comm));
}

Call Process::comm_free(int comm) {
    return create(CallType::COMM_FREE, Collective::comm_free(comm));
}

Call Process::allreduce(int count, int datatype, int op, int comm) {
    return create(CallType::ALLREDUCE, Collective::allreduce(count, datatype, op, comm));
}

Call Process::reduce(int count, int datatype, int op, int comm) {
    return create(CallType::REDUCE, Collective::reduce(count, datatype, op, comm));
}

Call Process::gather(int sendcount, int sendtype, int recvcount, int recvtype, int root, int comm) {
    return create(CallType::GATHER, Collective::gather(sendcount, sendtype, recvcount, recvtype, root, comm));
}

Call Process::scatter(int sendcount, int sendtype, int recvcount, int recvtype, int root, int comm) {
    return create(CallType::SCATTER, Collective::scatter(sendcount, sendtype, recvcount, recvtype, root, comm));
}

Call Process::scatterv(int sendtype, int recvcount, int recvtype, int root, int comm) {
    return create(CallType::SCATTERV, Collective::scatterv(sendtype, recvcount, recvtype, root, comm));
}

Call Process::gatherv(int sendcount, int sendtype, int recvtype, int root, int comm) {
    return create(CallType::GATHERV, Collective::gatherv(sendcount, sendtype, recvtype, root, comm));
}

Call Process::allgather(int sendcount, int sendtype, int recvcount, int recvtype, int comm) {
    return create(CallType::ALLGATHER, Collective::allgather(sendcount, sendtype, recvcount, recvtype, comm));
}

Call Process::allgatherv(int sendcount, int sendtype, int recvtype, int comm) {
    return create(CallType::ALLGATHERV, Collective::allgatherv(sendcount, sendtype, recvtype, comm));
}

Call Process::alltoall(int sendcount, int sendtype, int recvcount, int recvtype, int comm) {
    return create(CallType::ALLTOALL, Collective::alltoall(sendcount, sendtype, recvcount, recvtype, comm));
}

Call Process::alltoallv(int sendtype, int recvtype, int comm) {
    return create(CallType::ALLTOALLV, Collective::alltoallv(sendtype, recvtype, comm));
}

Call Process::scan(int count, int datatype, int op, int comm) {
    return create(CallType::SCAN, Collective::scan(count, datatype, op, comm));
}

Call Process::reduce_scatter(int datatype, int op, int comm) {
    return create(CallType::REDUCE_SCATTER, Collective::reduce_scatter(datatype, op, comm));
}

Call Process::finalize() {
    return create(CallType::FINALIZE);
}

Call Process::create(CallType call_type) {
    Call c;
    c.pid = pid;
    c.handle = curr_handle++;
    c.call_type = call_type;
    return c;
}

Call Process::create(CallType call_type, const Receive &recv) {
    Call c = create(call_type);
    c.recv = recv;
    return c;
}

Call Process::create(CallType call_type, const Send &send) {
    Call c = create(call_type);
    c.send = send;
    return c;
}

Call Process::create(CallType call_type, const Collective &col) {
    Call c = create(call_type);
    c.collective = col;
    return c;
}
