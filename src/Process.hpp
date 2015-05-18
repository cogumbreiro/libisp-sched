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

    Call irecv(int src);

    Call irecv(WInt src);

    Call recv(int count, int datatype, WInt src, WInt tag, int comm);

    Call send(int count, int datatype, int dest, int tag, int comm);

    Call ssend(int count, int datatype, int dest, int tag, int comm);

    Call isend(int count, int datatype, int dest, int tag, int comm);

    Call isend(int dest);

    Call rsend(int count, int datatype, int dest, int tag, int comm);

    Call wait(int request);

    Call barrier(int comm);

    Call finalize();

private:
    Call create(OpType call_type);

    Call create(OpType call_type, const Receive &recv);

    Call create(OpType call_type, const Send &send);
};

#endif
