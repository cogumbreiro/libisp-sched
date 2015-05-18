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
    Call c = create(OpType::BARRIER);
    c.collective.set(Field::Communicator, comm);
    return c;
}
/*
Call Process::barrier() {
    return barrier(0);
}*/

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
