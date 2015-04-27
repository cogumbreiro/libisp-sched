/*
 * Copyright (c) 2008-2009
 *
 * School of Computing, University of Utah,
 * Salt Lake City, UT 84112, USA
 *
 * and the Gauss Group
 * http://www.cs.utah.edu/formal_verification
 *
 * See LICENSE for licensing information
 */

/*
 * ISP: MPI Dynamic Verification Tool
 *
 * File:        Envelope.cpp
 * Description: Implements the various envelopes received from profiled code
 * Contact:     isp-dev@cs.utah.edu
 */

#include "Envelope.hpp"
#include "Options.hpp"
#include <sstream>
#include <iostream>
#include <stdlib.h>
#include <assert.h>

using std::unique_ptr;
using std::string;

unique_ptr<Envelope> CreateEnvelope (const string &buffer, int id, int order_id, bool to_expl) {
    if (buffer.empty()) {
        return nullptr;
    }

    // create result object
    unique_ptr<Envelope> env = std::make_unique<Envelope>();
    env->issue_id = -1;
    env->in_exall = to_expl;

    std::istringstream iss;
    iss.str(buffer);
    iss >> env->index;

    //read in the function name and parse it to generate the function id
    string func_name;
    iss >> func_name;
    env->func = func_name;
    env->func_id = name2id::getId(func_name);
    env->id = id;
    env->order_id = order_id;
    env->comm = -2;

    //read in the display name
    iss >> env->display_name;

    if (env->func_id != LEAK && env->func_id != PCONTROL){
       iss >> env->data_type;
       env->typesMatch = true;
    }

    /*
     * Parse format: ... <filename length> <file name> <line number> ...
     */
    int filename_length;
    iss >> filename_length;
    iss.ignore (); // ignore space
    char *filename = new char[filename_length + 1];
    iss.get (filename, filename_length + 1);
    env->filename.insert(0, filename);
    delete [] filename;
    iss >> env->linenumber;

    /*
     * Update the structure based on the function.
     */
    switch (env->func_id) {

    case ASSERT: {
            // Get the assertion text and function name.
            int assertion_length;
            iss >> assertion_length;
            iss.ignore (); // ignore space
            char *assertion = new char[assertion_length + 1];
            iss.get (assertion, assertion_length + 1);
            env->display_name.clear ();
            env->display_name.insert (0, assertion);
            delete assertion;
            iss >> env->func;
            break;
        }

    case ISEND: {
            string dest;
            iss >> dest >> env->stag >> env->count >> env->comm;
            if (dest == "MPI_ANY_SOURCE") {
                env->dest = WILDCARD;
                env->dest_wildcard = true;
            } else {
                env->dest = atoi (dest.c_str ());
                env->dest_wildcard = false;
            }
            break;
        }
    case SSEND:
	case RSEND:
    case SEND: {
            string dest;
            iss >> dest >> env->stag >> env->comm;
            if (dest == "MPI_ANY_SOURCE") {
                env->dest = WILDCARD;
                env->dest_wildcard = true;
            } else {
                env->dest = atoi (dest.c_str ());
                env->dest_wildcard = false;
            }
            break;
        }

    case IRECV: {
            string src;
            iss >> src >> env->rtag >> env->count >>  env->comm;
            if (src == "MPI_ANY_SOURCE") {
                env->src = WILDCARD;
                env->src_wildcard = true;
            } else {
                env->src = atoi (src.c_str ());
                env->src_wildcard = false;
            }
            if (env->rtag < 0)
                env->rtag = WILDCARD;
            break;
        }
    case IPROBE:
    case PROBE:
    case RECV: {
            string src;
            iss >> src >> env->rtag >> env->comm;
            if (src == "MPI_ANY_SOURCE") {
                env->src = WILDCARD;
                env->src_wildcard = true;
            } else {
                env->src = atoi (src.c_str ());
                env->src_wildcard = false;
            }
            if (env->rtag < 0)
                env->rtag = WILDCARD;
            break;
        }

    case SENDRECV:
        iss >> env->dest >> env->stag >> env->src >>
        env->rtag >> env->comm;
        if (env->rtag < 0) env->rtag = WILDCARD;
        break;

    case BCAST:
    case SCATTER:
    case GATHER:
    case SCATTERV:
    case GATHERV:
    case REDUCE: {
            iss >> env->nprocs;
            iss >> env->count;
            if (iss.fail ()) {
                env->count = -1;
            }
            iss >> env->comm;
            break;
        }

    case ALLGATHER:
    case ALLGATHERV:
    case ALLTOALL:
    case ALLTOALLV:
    case SCAN:
	case EXSCAN:
    case BARRIER:
    case ALLREDUCE:
    case REDUCE_SCATTER:
    case CART_CREATE:
    case COMM_CREATE:
    case COMM_DUP:
    case COMM_FREE:
        iss >> env->nprocs >> env->count >> env->comm;
        break;

    case COMM_SPLIT:
        iss >> env->nprocs >> env->count >> env->comm_split_color >> env->comm;
        break;

    case WAIT:
    case TEST:
        iss >> env->count;
        env->req_procs.push_back (env->count);
        break;

    case WAITALL:
    case TESTALL:
    case WAITANY:
    case TESTANY:
        iss >> env->count;
        for (int i = 0; i < env->count ; i++) {
            int req;
            iss >> req;
            env->req_procs.push_back (req) ;
        }
        break;

    case PCONTROL:
        int pcontrol_mode;
        iss >> pcontrol_mode;
        env->stag = pcontrol_mode;  // NOTE: we store the mode information in "stag"!!!!
        break;

    case LEAK:
    case ABORT:
    case FINALIZE:
        break;
    }
    return env;
}

Envelope::Envelope() {
    dest = 0;
    dest_wildcard = false;
    src = 0;
    src_wildcard = false;
}

bool Envelope::operator== (const Envelope &e) const {
    if (e.func != func) {
        return false;
    }
    switch (e.func_id) {
    case ASSERT:
        return (display_name == e.display_name);

    case BARRIER:
    case BCAST:
    case SCATTER:
    case GATHER:
    case SCATTERV:
    case GATHERV:
    case ALLGATHER:
    case ALLGATHERV:
    case ALLTOALL:
    case ALLTOALLV:
    case SCAN:
	case EXSCAN:
    case ALLREDUCE:
    case REDUCE:
    case REDUCE_SCATTER:
    case CART_CREATE:
    case COMM_CREATE:
    case COMM_DUP:
    case COMM_SPLIT:
    case COMM_FREE:
    case WAIT:
    case TEST:
    case WAITANY:
    case TESTANY:
    case WAITALL:
    case TESTALL:
        return (count == e.count);

    case SEND:
    case SSEND:
    case ISEND:
	case RSEND:
        return (dest==e.dest && e.stag == stag);

    case IRECV:
    case RECV:
    case PROBE:
    case IPROBE:
        return (e.src == src && e.rtag == rtag);

    case ABORT:
    case FINALIZE:
        return true;

    case LEAK:
        return (e.filename == filename && e.linenumber == linenumber &&
                e.count == count);
    case PCONTROL:
        return (e.stag == stag);
    }
    return false;
}

bool Envelope::operator!= (const Envelope &e) const {
    return !((*this) == e);
}
