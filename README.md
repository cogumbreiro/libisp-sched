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

# Design

The scheduler is a mediator between the program and the runtime.

1. Whenever a process in the program invokes an MPI call it should be enqueued.

2. Whenever all processes are blocked provide the queue of MPI calls to the
   scheduler. The scheduler returns a list of matchsets. A matchset is a batch
   of calls.

2.1. If the list of matchsets is empty, then declare a deadlock.

2.2. Otherwise, for each matchset fork an execution state and let the runtime
     execute that matchset.

# Example

Include the generator (that yields the matchsets) and the process (a factory to build call objects).
```
#include "Generator.hpp"
#include "Process.hpp"
```

Create a schedule, set the total number of participants (processes)
and also set how many participants the default communcitator `COMM_WORLD` has.
```
int COMM_WORLD = 0;
Schedule s;
s.procs = 3;
s.participants[COMM_WORLD] = 3;
```

Now create three processes and schedule their calls:
```
Process P0(0), P1(1), P2(2);
// P0:
Call c1 = P0.isend(P1.pid);
s.calls.push_back(c1);
Call c2 = P0.barrier(COMM_WORLD);
s.calls.push_back(c2);
// P1:
Call c4 = P1.irecv(WILDCARD);
s.calls.push_back(c4);
Call c5 = P1.barrier(COMM_WORLD);
s.calls.push_back(c5);
// P2:
Call c7 = P2.barrier(COMM_WORLD);
s.calls.push_back(c7);
```
Now given a schedule generate all possible matchsets, which in this case will be three.
```
auto ms = get_match_sets(s);
// the program is *deterministic*, so only one trace is allowed
REQUIRE(1 == ms.size());
auto tmp = *ms.begin();
auto inter = tmp.toVector();
REQUIRE(inter.size() == 3);
REQUIRE(inter[0] == c2); // P0:Barrier
REQUIRE(inter[1] == c5); // P1:Barrier
REQUIRE(inter[2] == c7); // P2:Barrier
```
The full test case is available at [`TestExamples.cpp`](src/TestExamples.cpp).

[ISP]: http://formalverification.cs.utah.edu/ISP-Release/
