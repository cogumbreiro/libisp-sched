#ifndef _CALL_TYPE_HPP
#define _CALL_TYPE_HPP

enum class CallType {
    SEND,
	RSEND,
    SSEND,
    SEND_INIT,
    RECV,
    PROBE,
    IPROBE,
    SENDRECV,
    RECV_INIT,
    BARRIER,
    BCAST,
    SCATTER,
    GATHER,
    SCATTERV,
    GATHERV,
    ALLGATHER,
    ALLGATHERV,
    ALLTOALL,
    ALLTOALLV,
    SCAN,
    REDUCE,
    REDUCE_SCATTER,
    ISEND,
    IRECV,
    START,
    STARTALL,
    WAIT,
    TEST,
    ALLREDUCE,
    WAITALL,
    TESTALL,
    WAITANY,
    TESTANY,
    REQUEST_FREE,
    CART_CREATE,
    COMM_CREATE,
    COMM_DUP,
    COMM_SPLIT,
    COMM_FREE,
    ABORT,
    FINALIZE,
    PCONTROL,
	EXSCAN
};


bool is_send(CallType call_type);

bool is_recv(CallType call_type);

bool is_collective(CallType call_type);

bool is_probe(CallType call_type);

bool is_wait(CallType call_type);

bool is_test(CallType call_type);

bool is_blocking(CallType call_type);

#endif
