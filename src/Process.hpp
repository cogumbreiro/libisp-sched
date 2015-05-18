#ifndef _PROCESS_HPP
#define _PROCESS_HPP

#include "WInt.hpp"
#include "Call.hpp"

/**
 * @brief A factory of Call objects, should be used per process.
 */
struct Process {
    int pid;

    int curr_handle;

    Process(int pid);

    Call irecv(int count, int datatype, WInt src, WInt tag, int comm);

    Call recv(int count, int datatype, WInt src, WInt tag, int comm);

    Call send(int count, int datatype, int dest, int tag, int comm);

    Call ssend(int count, int datatype, int dest, int tag, int comm);

    Call isend(int count, int datatype, int dest, int tag, int comm);

    Call rsend(int count, int datatype, int dest, int tag, int comm);

    Call wait(int request);

    Call barrier(int comm);

    Call bcast(int count, int datatype, int root, int comm);

    Call cart_create();

    Call comm_create(int comm);

    Call comm_dup(int comm);

    Call comm_split(int comm, int color, int key);

    Call comm_free(int comm);

    Call allreduce(int count, int datatype, int op, int comm);

    Call reduce(int count, int datatype, int op, int comm);

    Call gather(int sendcount, int sendtype, int recvcount, int recvtype, int root, int comm);

    Call scatter(int sendcount, int sendtype, int recvcount, int recvtype, int root, int comm);

    Call scatterv(int sendtype, int recvcount, int recvtype, int root, int comm);

    Call gatherv(int sendcount, int sendtype, int recvtype, int root, int comm);

    Call allgather(int sendcount, int sendtype, int recvcount, int recvtype, int comm);

    Call allgatherv(int sendcount, int sendtype, int recvtype, int comm);

    Call alltoall(int sendcount, int sendtype, int recvcount, int recvtype, int comm);

    Call alltoallv(int sendtype, int recvtype, int comm);

    Call scan(int count, int datatype, int op, int comm);

    Call reduce_scatter(int datatype, int op, int comm);

    Call finalize();

private:
    Call create(OpType call_type);

    Call create(OpType call_type, const Receive &recv);

    Call create(OpType call_type, const Send &send);

    Call create(OpType call_type, const Collective &);
};

#endif
