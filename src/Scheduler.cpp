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
 * File:        Scheduler.cpp
 * Description: Scheduler that implements the process scheduling
 * Contact:     isp-dev@cs.utah.edu
 */

#include "Scheduler.hpp"
#include <sys/types.h>
#ifndef WIN32
#include <sys/select.h>
#include <sys/time.h>
#endif
#include <fstream>
#include <sstream>
#include <vector>
/* == fprs begin == */
#include <ctime>
/* == fprs end == */
#include "InterleavingTree.hpp"
/* == fprs begin == */
#define ISP_START_SAMPLING 2
#define ISP_END_SAMPLING 3
/* == fprs end == */

#ifdef _MSC_VER
#pragma warning( disable : 4127 ) // warning C4127: conditional expression is constant
#endif

Scheduler * Scheduler::_instance = NULL;
/*
 * Initialize network variables.
 */
std::string Scheduler::_num_procs = "";
std::string Scheduler::_fname = "";
std::vector<std::string> Scheduler::_fargs = std::vector<std::string>();
std::string Scheduler::_port = "";
std::ofstream Scheduler::_logfile;
std::stringstream Scheduler::_mismatchLog;//CGD
std::string Scheduler::_server = "";
bool Scheduler::_send_block = false;
bool Scheduler::_mpicalls = false;
bool Scheduler::_verbose = false;
bool Scheduler::_quiet = false;
bool Scheduler::_unix_sockets = false;
int  Scheduler::_report_progress = 0;
bool Scheduler::_fib = false;
bool Scheduler::_openmp = false;
bool Scheduler::_env_only = false;
bool Scheduler::_probed = false;
bool Scheduler::_batch_mode = false;
int  Scheduler::interleavings = 0;
te_Exp_Mode  Scheduler::_explore_mode = EXP_MODE_ALL;
int  Scheduler::_explore_some = 0;
std::vector<int>* Scheduler::_explore_all = NULL;
std::vector<int>* Scheduler::_explore_random = NULL;
std::vector<int>* Scheduler::_explore_left_most = NULL;
bool Scheduler::_stop_at_deadlock = false;
bool Scheduler::_just_dead_lock = false;
bool Scheduler::_deadlock_found = false;
bool Scheduler::_param_set = false;
int  Scheduler::_errorno = 0;
bool Scheduler::_debug = false;
bool Scheduler::_no_ample_set_fix = false;
unsigned Scheduler::_bound = 0;
bool Scheduler::_limit_output = false;
/* == fprs start == */
bool Scheduler::_fprs;
/* == fprs end == */

fd_set Scheduler::_fds;
SOCKET Scheduler::_lfd = 0;
std::map <SOCKET, int> Scheduler::_fd_id_map;

/*
 * Queues maintained by the Scheduler
 */

std::vector <MpiProc *> Scheduler::_runQ;
Node *Scheduler::root = NULL;

Scheduler * Scheduler::GetInstance () {

    assert (_param_set);

    /*
     * If Instance is already created, just return it.
     */
    if (_instance != NULL) {
        return _instance;
    }
    _instance = new Scheduler ();
    if (_errorno != 0) {
        delete _instance;
        exit (_errorno);
    }
    return _instance;
}

void Scheduler::SetParams (std::string port, std::string num_clients,
                           std::string server, bool send_block,
                           std::string fname, std::string logfile,
                           std::vector<std::string> fargs,
                           bool mpicalls, bool verbose, bool quiet,
                           bool unix_sockets, int report_progress,
                           bool fib, bool openmp, bool env_only,
                           bool batch_mode, bool stop_at_deadlock,
                           te_Exp_Mode explore_mode, int explore_some,
                           std::vector<int> *explore_all,
                           std::vector<int> *explore_random,
                           std::vector<int> *explore_left_most,
                           bool debug, bool no_ample_set_fix,
                           unsigned bound, bool limit_output
/* == fprs start == */
, bool fprs
/* == fprs end == */
) {//CGD

    _param_set = true;
    _port = port;
    _num_procs = num_clients;
    _server = server;
    _send_block = send_block;
    _fname = fname;
    _fargs = fargs;
    _mpicalls = mpicalls;
    _verbose = verbose;
    _quiet = quiet;
    _unix_sockets = unix_sockets;
    _report_progress = report_progress;
    _fib = fib;
    _openmp = openmp;
    _env_only = env_only;
    _batch_mode = batch_mode;
    _stop_at_deadlock = stop_at_deadlock;
    _explore_mode = explore_mode;
    _explore_some = explore_some;
    _explore_all = explore_all;
    _explore_random = explore_random;
    _explore_left_most = explore_left_most;
    _debug = debug;
    _no_ample_set_fix = no_ample_set_fix;
    _bound = bound;
    _limit_output = limit_output;//CGD
/* == fprs start == */
    _fprs = fprs;
/* == fprs end == */

    /* open the logfile for writing */
    _logfile.open (logfile.c_str());
    if (logfile != "" && ! _logfile.is_open()) {
        if (!quiet) {
            std::cout << "Unable to open file " << logfile << "\n";
        }
        exit (20);
    } else {
        _logfile << num_clients << "\n";
    }
}

Scheduler::Scheduler () :
        ServerSocket (atoi(_port.c_str()),atoi(_num_procs.c_str()),
                      _unix_sockets, _batch_mode), order_id(1) {

    _errorno = Start ();
    if (_errorno != 0) {
        if (!_quiet) {
            switch (_errorno) {
                case 13:
                    std::cout << "Server unable to open Socket!\n";
                    break;
                case 14:
                    std::cout << "Server unable to bind to port. The port may already be in use.\n";
                    break;
                case 15:
                    std::cout << "Server unable to listen on the socket\n";
                    break;
            }
        }
        return;
    }

    StartClients ();

    _errorno = Accept ();
    if (_errorno != 0) {
        if (!_quiet) {
            switch (_errorno) {
                case 16:
                    std::cout << "Server unable to accept\n";
                    break;
                case 17:
                    std::cout << "Error reading from socket in server\n";
                    break;
            }
        }
        return;
    }

    FD_ZERO (&_fds);
    _lfd = 0;
    for (int i = 0 ; i < atoi (_num_procs.c_str ()); i++) {
        SOCKET fd = GetFD (i);
        FD_SET (fd, &_fds);
        _fd_id_map[fd] = i;
        if (fd > _lfd) {
            _lfd = fd;
        }
    }

    it  = new ITree ( new Node (atoi(_num_procs.c_str ())), ProgName());

    /*
     * Intially all processes are ready to run. Add
     * all procs to the list.
     */

    for (int i = 0 ; i < atoi (_num_procs.c_str ()); i++) {
        _runQ.push_back (new MpiProc (i));
    }

/* == fprs bedin == */
    it->_is_exall_mode = (bool*) malloc(sizeof(bool)*atoi(_num_procs.c_str()));
    for (int i = 0 ; i < atoi(_num_procs.c_str()) ; i++) it->_is_exall_mode[i] = false;
/* == fprs end == */
}

std::string Scheduler::ProgName () {

    std::string::size_type pos = _fname.find_last_of ("/", _fname.size ()-1);
    if (pos == std::string::npos) {
        return _fname;
    }
    return _fname.substr (pos+1, _fname.size ()-1);
}

void Scheduler::StartClients () {
    std::vector<const char *> cmd;

    interleavings++;
    const char *use_mpirun_env = getenv("USE_MPIRUN");
    bool use_mpirun = use_mpirun_env && !strcmp(use_mpirun_env, "true");

    if (_batch_mode) {
        std::cout << "Please launch the clients on the cluster with the following command:" << std::endl;
        if (use_mpirun) {
            std::cout << "    mpirun ";
        } else {
            std::cout << "    mpiexec ";
        }
        std::cout << "-n " << _num_procs << " "
                  << "\"" << _fname << "\" "
                  << "0 "
                  << _port << " "
                  << _num_procs << " "
                  << _server << " "
                  << (_send_block ? "1" : "0") << " ";
        for (int i = 0; i < int(_fargs.size()); i++) {
            std::cout << _fargs[i] << " ";
        }
        std::cout << std::endl;
        return;
    }

    if (use_mpirun) {
        cmd.push_back("mpirun");
    } else {
        cmd.push_back("mpiexec");
    }
    cmd.push_back("-n");
    cmd.push_back((const char *)_num_procs.c_str());

    if (_env_only) {
        cmd.push_back("-env");
        cmd.push_back("ISP_USE_ENV");
        cmd.push_back("1");
        cmd.push_back("-env");
        cmd.push_back("ISP_HOSTNAME");
#ifndef WIN32
        if (_unix_sockets) {
            cmd.push_back((const char *)GetSocketFile ().c_str());
            cmd.push_back("-env");
            cmd.push_back("ISP_UNIX_SOCKETS");
            cmd.push_back("1");
        }
        else
#endif
        {
            cmd.push_back((const char *)_server.c_str());
            cmd.push_back("-env");
            cmd.push_back("ISP_UNIX_SOCKETS");
            cmd.push_back("0");
            cmd.push_back("-env");
            cmd.push_back("ISP_PORT");
            cmd.push_back((const char *)_port.c_str());
        }
        cmd.push_back("-env");
        cmd.push_back("ISP_NUMPROCS");
        cmd.push_back((const char *)_num_procs.c_str());
        cmd.push_back("-env");
        cmd.push_back("ISP_SENDBLOCK");
        cmd.push_back(_send_block ? "1" : "0");
    }

    cmd.push_back((const char *)_fname.c_str());
    if (!_env_only) {
#ifndef WIN32
        if (_unix_sockets) {
            cmd.push_back("1");
            cmd.push_back(GetSocketFile ().c_str());
        }
        else
#endif
        {
            cmd.push_back("0");
            cmd.push_back((const char *)_port.c_str());
        }
        cmd.push_back((const char *)_num_procs.c_str());
        cmd.push_back((const char *)_server.c_str());
        cmd.push_back(_send_block ? "1" : "0");

        std::stringstream ss_interleaving;
        ss_interleaving << interleavings;
        cmd.push_back(ss_interleaving.str().c_str());
    }

    for (int i = 0; i < int(_fargs.size()); i++) {
        cmd.push_back((const char *)_fargs[i].c_str());
    }
    cmd.push_back(NULL);

#ifndef WIN32
    int gpid = fork ();
    /*
     * Child process must now exec
     */
    if (gpid == 0) {
        if (execvp (cmd[0], (char* const*) &cmd[0]) < 0) {
            cmd[0] = "mpirun";
            if (execvp (cmd[0], (char* const*) &cmd[0]) < 0) {
                if (!_quiet) {
                    std::cout <<"Unable to Start Clients using exec!\n";
                }
                exit (12);
            }
        }
    } else if (gpid < 0) {
        perror(0);
        exit(1);
    }
    _pid = gpid;
#else
    std::string filename("\"" + _fname + "\"");
    DWORD execvp(const char *file, char const **argv);
    if ((_pid = execvp (NULL, &cmd[0])) == 0) {
        if (!_quiet) {
            std::cout <<"Unable to Start Clients using exec!\n";
        }
        exit (12);
    }
#endif

    if (!_quiet && !_limit_output) {//CGD
        std::cout << "Started Process: " << _pid << std::endl;
    }
}

int Scheduler::Restart () {
    int result;

    ServerSocket::Restart ();
    StartClients ();
    if ((result = Accept ()) != 0) {
        if (!_quiet) {
            switch (result) {
                case 16:
                    std::cout << "Server unable to accept\n";
                    break;
                case 17:
                    std::cout << "Error reading from socket in server\n";
                    break;
            }
        }
        exit (result);
    }
    for (int i = 0 ; i < atoi (_num_procs.c_str ()); i++) {
        _runQ[i]->_read_next_env = true;
/* == fprs begin == */
        it->_is_exall_mode[i] = false;
/* == fprs end == */
    }
    it->ResetMatchingInfo();
    return 0;
}

void Scheduler::StartMC () {
    /*
     * First generate one single interleaving.
     * Use timeouts for this incase the processes
     * have deadlocked!
     */
/* == fprs begin == */
    unsigned ran_seed = time(0);
/* == fprs end == */
    for (;;) {
        if (!_quiet) {
            std::cout << "INTERLEAVING :" << interleavings << "\n";
            fflush(stdout);
        }
/* == fprs begin == */
    if (Scheduler::_fprs) {
        srand(ran_seed);
        Scheduler::_explore_mode = EXP_MODE_RANDOM;
    }
/* == fprs end == */
        generateFirstInterleaving ();
/* == fprs begin == */
    if (Scheduler::_fprs) {
        Scheduler::_explore_mode = EXP_MODE_ALL;
    }
/* == wfchiag end == */
#ifdef FIB
        if (_fib) {
            it->findInterCB ();
            it->FindRelBarriers ();
        }
#endif
        it->GetCurrNode ()->PrintLog ();

        //CGD print Mismatched Types
/* == fprs start == */
        //if (_fprs == false) {
        //it->printTypeMismatches();
        //}
/* == fprs end == */

        if (Scheduler::_just_dead_lock) {
          Scheduler::_logfile << Scheduler::interleavings << " DEADLOCK\n";
          Scheduler::_just_dead_lock = false;
          Scheduler::_deadlock_found = true;
        }
        bool outputused = false;
        std::stringstream outputmsg;
        outputmsg << "-----------------------------------------" << std::endl;
        if (_verbose) {
            outputmsg << *(it->GetCurrNode ());
            outputused = true;
        } else if (!_quiet && !_limit_output) {//CGD
            std::vector<TransitionList *>::iterator iter, iter_end;
            iter = it->GetCurrNode ()->_tlist.begin ();
            iter_end = it->GetCurrNode ()->_tlist.end ();
            for (; iter != iter_end; iter++) {
                bool output = false;
                if ((*iter)->_leaks_count != 0) {
                    output = true;
                    outputmsg << "Rank " << (*iter)->GetId () << ": ";
                    outputmsg << (*iter)->_leaks_count << " resource leaks detected.";
                }
                if (_mpicalls) {
                    if (!false) {
                        outputmsg << "Rank " << (*iter)->GetId () << ": ";
                        output = true;
                    }
                    outputmsg << (*iter)->_tlist.size () << " MPI calls trapped.";
                }
                if (output) {
                    outputmsg << std::endl;
                }
                outputused = outputused || output;
            }
        }
        if (it->NextInterleaving ()) {
            if (Scheduler::_deadlock_found && Scheduler::_stop_at_deadlock) {
                std::cout << "Verification stopped. There are still more interleavings to explore\n";
                std::cout << "To continue exploring, please re-run and do not set stop-at-deadlock flag\n";
                ExitMpiProcessAndWait(true);
                exit(1);
            }
            if (outputused) {
                outputmsg << "-----------------------------------------" << std::endl;
                std::cout << outputmsg.str();
            }
            it->resetDepth();
#ifdef FIB
            if (_fib) {
                /*
                 * At the start of next interleaving clear the taken ample moves
                 * in the previous interleaving.
                 */
                it->al_curr.clear();
            }
#endif
            Restart ();
            //std::cout << "THERE ARE MORE INTERLEAVINGS!!!\n";
        } else {
            ExitMpiProcessAndWait (true);
            if (!_quiet && !_limit_output) {//CGD
                std::string explore_mode = "";
                if (Scheduler::_explore_mode == EXP_MODE_ALL)
                  explore_mode = "All Relevant Interleavings";
                else if (Scheduler::_explore_mode == EXP_MODE_RANDOM)
                  explore_mode = "Random Choice";
                else if (Scheduler::_explore_mode == EXP_MODE_LEFT_MOST)
                  explore_mode = "First Available Choice";

                if (Scheduler::_deadlock_found)
                    outputmsg << "ISP detected deadlock!!!" << std::endl;
                else
                    outputmsg << "ISP detected no deadlocks!" << std::endl;

                outputmsg << "Total Explored Interleavings: " << interleavings << std::endl;
                outputmsg << "Interleaving Exploration Mode: " << explore_mode << std::endl;
                outputmsg << "-----------------------------------------" << std::endl;
                std::cout << outputmsg.str();
            }
#ifdef FIB
            if (_fib) {
                it->printRelBarriers ();
            }
            delete it->last_node;
#endif
			if (Scheduler::_deadlock_found) {
				exit(1);
			}
            return;
        }
    }
}

void Scheduler::generateFirstInterleaving () {
    struct timeval tv;
    Transition *t;
    tv.tv_usec = 0;
    tv.tv_sec = 2;
    int count = atoi(_num_procs.c_str ());
    int nprocs = count;

    while (count) {
        for (int i = 0 ; i < nprocs; i++) {
            if (_runQ[i]->_read_next_env) {
                t = getTransition(i);
                // IMPORTANT NOTE: the constant ISP_START_SAMPLING and
                // ISP_END_SAMPLEING here should be corresponded to the
                // ISP_START_SAMPLING and ISP_END_SAMPLING defined in isp.h
                if (_fprs) {
                    if (t->GetEnvelope()->func_id == PCONTROL && t->GetEnvelope()->stag == ISP_START_SAMPLING) {
                        it->_is_exall_mode[i] = true;
                    }
                    if (t->GetEnvelope()->func_id == PCONTROL && t->GetEnvelope()->stag == ISP_END_SAMPLING) {
                        it->_is_exall_mode[i] = false;
                    }
                }
                if (!(it->GetCurrNode ())->_tlist[i]->AddTransition (t)) {
                    ExitMpiProcessAndWait (true);

                    if (!Scheduler::_quiet) {
                        /* ISP expects the same transitions on restart
                         * to ensure the correctness of DPOR
                         * backtracking - If the program changes its
                         * control flow path upon restarting, ISP will
                         * not work */
                        std::cout << "ERROR! TRANSITIONS ON RESTARTS NOT SAME!!!!\n";
                    }
                    exit (21);
                }

                /* Some memory leak can happen here if we're not in
                   the first interleaving. T has a "NEW" envelope
                   which is never freed When this leak is fixed,
                   please remove this comment */
                if (! t->GetEnvelope ()->isBlockingType ()) {
                    Send (i, goback);
                } else {
                    _runQ[i]->_read_next_env = false;
                }
                if (t->GetEnvelope ()->func_id == FINALIZE) {
                    _runQ[i]->_read_next_env = false;
                    count--;
                }
                delete t;
            }
        }
        std::list <int> l;
        std::list <int>::iterator iter;
        std::list <int>::iterator iter_end;
        while (!hasMoreEnvelopeToRead ()) {
            if (it->CHECK (*this, l) == 1)
                return;
            iter_end = l.end();

            /* This takes advantage of the fact that if we have
               PROBE/IPROBE, the size of l has to be less than 2, in
               which the first one is the send, the second one is the
               PROBE/IPROBE */
            /* In the case of PROBE/IPROBE, the iprobe call is the
               only call that should be allowed to go ahead */
            if (Scheduler::_probed) {
                assert (l.size() <= 2);
                _runQ[l.back()]->_read_next_env = true;
                Scheduler::_probed = false;
            } else {
                for (iter = l.begin (); iter != iter_end; iter++) {
                    _runQ[*iter]->_read_next_env = true;

                }
            }
        }

    }
    // Reached and of interleaving without any deadlocks - update InterCB
    it->ProcessInterleaving();
}

bool Scheduler::hasMoreEnvelopeToRead () {

    for (int  i = 0 ; i < atoi (_num_procs.c_str ()); i++) {
        if (_runQ[i]->_read_next_env) {
            return true;
        }
    }
    return false;
}

Transition *Scheduler::getTransition (int id) {

    Envelope *e = NULL;
    char    buffer[BUFFER_SIZE];

    do {
        memset(buffer, '\0', BUFFER_SIZE);
        Receive (id, buffer, BUFFER_SIZE);

/* == fprs start == */
        e = CreateEnvelope (buffer, id, order_id++, it->_is_exall_mode[id]);
/* == fprs end == */
        if (!e) {
            it->GetCurrNode ()->PrintLog ();
            ExitMpiProcessAndWait (true);
            if (!_quiet) {
                std::cout << *(it->GetCurrNode ()) << "\n";
                std::cout << "Receiving empty buffer - Terminating ...\n";
            }
            exit (22);
        }
        //DS( std::cout << " [getTransition] e=" << e << " e->func_id=" << e->func_id << "\n"; )
        if (e->func_id == LEAK) {
            std::stringstream& leaks = it->GetCurrNode ()->_tlist[id]->_leaks_string;
            it->GetCurrNode ()->_tlist[id]->_leaks_count++;
            leaks << Scheduler::interleavings << " " << id << " Leak ";
            leaks << e->display_name << " { } { } Match: -1 -1 File: ";
            leaks << e->filename.length () << " " << e->filename << " " << e->linenumber << std::endl;
            Send (id, goback);
            order_id--;
        }
    } while (e->func_id == LEAK);

    bool error = false;
    switch (e->func_id) {

    case ASSERT:
        it->GetCurrNode ()->PrintLog ();
        _logfile << interleavings << " "
                 << e->id << " "
                 << "ASSERT "
                 << "Message: " << e->display_name.length() << " " << e->display_name << " "
                 << "Function: " << e->func.length() << " " << e->func << " "
                 << "File: " << e->filename.length() << " " << e->filename << " " << e->linenumber << "\n";
        ExitMpiProcessAndWait (true);
        if (!_quiet) {
            std::cout << "-----------------------------------------" << std::endl;
            std::cout << *(it->GetCurrNode ()) << std::endl;
            std::cout << "Program assertion failed!" << std::endl;
            std::cout << "Rank " << e->id << ": " << e->filename << ":" << e->linenumber
                      << ": " << e->func << ": " << e->display_name << std::endl;
            std::cout << "Killing program " << ProgName() << std::endl;
            std::cout << "-----------------------------------------" << std::endl;
            std::cout.flush();
        }
        exit (3);
        break;

    case ABORT:
        it->GetCurrNode()->_tlist[id]->AddTransition(new Transition(e));
        it->GetCurrNode ()->PrintLog ();
        ExitMpiProcessAndWait (true);
        if (!_quiet) {
            std::cout << "-----------------------------------------" << std::endl;
            std::cout << *(it->GetCurrNode ()) << std::endl;
            std::cout << "MPI_Abort on rank " << e->id << " caused abort on all ranks." << std::endl;
            std::cout << "Killing program " << ProgName() << std::endl;
            std::cout << "-----------------------------------------" << std::endl;
            std::cout.flush();
        }
        exit (4);
        break;

    case SEND:
    case ISEND:
    case SSEND:
    case RSEND:
        if (!e->dest_wildcard &&
                (e->dest < 0 || e->dest >= atoi (_num_procs.c_str ()))) {
            error = true;
        }
        break;
    case IRECV:
    case IPROBE:
    case PROBE:
    case RECV:
        if (!e->src_wildcard &&
                (e->src < 0 || e->src >= atoi (_num_procs.c_str ()))) {
            error = true;
        }
        break;
    case SCATTER:
    case GATHER:
    case SCATTERV:
    case GATHERV:
    case BCAST:
    case REDUCE:
        if (e->count < 0 || e->count >= atoi (_num_procs.c_str ())) {
            error = true;
        }
        break;
    }
    if (error) {
        it->GetCurrNode ()->_tlist[id]->AddTransition(new Transition(e));
        it->GetCurrNode ()->PrintLog ();
        ExitMpiProcessAndWait (true);
        if (!_quiet) {
            std::cout << "-----------------------------------------" << std::endl;
            std::cout << *(it->GetCurrNode ()) << std::endl;
            std::cout << "Rank " << id << ": " << "Invalid rank in MPI_" << e->func << " at "
                      << e->filename << ":" << e->linenumber << std::endl;
            std::cout << "Killing program " << ProgName() << std::endl;
            std::cout << "-----------------------------------------" << std::endl;
            std::cout.flush();
        }
        exit (5);
    }

    return new Transition(e);
}
