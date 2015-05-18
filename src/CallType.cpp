#include "CallType.hpp"

bool is_send(CallType call_type) {
    return call_type == CallType::SSEND
            || call_type == CallType::SEND
            || call_type == CallType::RSEND
            || call_type == CallType::ISEND;
}

bool is_recv(CallType call_type) {
    return call_type == CallType::IRECV
            || call_type == CallType::RECV
            || call_type == CallType::PROBE
            || call_type == CallType::IPROBE;
}

bool is_collective(CallType call_type) {
    return call_type == CallType::BARRIER
            || call_type == CallType::BCAST
            || call_type == CallType::CART_CREATE
            || call_type == CallType::COMM_CREATE
            || call_type == CallType::COMM_DUP
            || call_type == CallType::COMM_SPLIT
            || call_type == CallType::COMM_FREE
            || call_type == CallType::ALLREDUCE
            || call_type == CallType::REDUCE
            || call_type == CallType::GATHER
            || call_type == CallType::SCATTER
            || call_type == CallType::GATHERV
            || call_type == CallType::SCATTERV
            || call_type == CallType::ALLGATHER
            || call_type == CallType::ALLGATHERV
            || call_type == CallType::ALLTOALL
            || call_type == CallType::ALLTOALLV
            || call_type == CallType::SCAN
            || call_type == CallType::REDUCE_SCATTER;
}

bool is_probe(CallType call_type) {
    return call_type == CallType::PROBE
            || call_type == CallType::IPROBE;
}

bool is_wait(CallType call_type) {
    return call_type == CallType::WAIT
            || call_type == CallType::WAITANY
            || call_type == CallType::WAITALL;
}

bool is_test(CallType call_type) {
    return call_type == CallType::TEST
            || call_type == CallType::TESTANY
            || call_type == CallType::TESTALL;
}

bool is_blocking(CallType call_type) {
    return call_type == CallType::RECV
            || call_type == CallType::SSEND
            || call_type == CallType::FINALIZE
            || is_probe(call_type)
            || is_test(call_type)
            || is_wait(call_type)
            || is_collective(call_type);
}
