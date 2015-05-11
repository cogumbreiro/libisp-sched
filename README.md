# libisp-sched: ISP's scheduler

A clean room implementation of [ISP]'s scheduler.

Download and initialize the project:

```bash
git clone --recursive https://github.com/cogumbreiro/libisp-sched/
cmake .
```

Compile:
```
make
```

Test:
```
make test
```

# Usage

The scheduler is a mediator between the program and the runtime.

1. Whenever a process in the program invokes an MPI call it should be enqueued.

2. Whenever all processes are blocked provide the queue of MPI calls to the
   scheduler. The scheduler returns a list of matchsets. A matchset is a batch
   of calls.

2.1. If the list of matchsets is empty, then declare a deadlock.

2.2. Otherwise, for each matchset fork an execution state and let the runtime
     execute that matchset.

To generate the matchsets given the enqueued calls use the function below,
located in `Generator.hpp`.
```C++
vector<MatchSet> get_match_sets(set<Call> & enabled);
```

[ISP]: http://formalverification.cs.utah.edu/ISP-Release/
