Multithreading, Parallelism, and Concurrency
============================================

The term ***multithreading*** refers to computing with multiple threads
of control. Once created, a thread performs a computation by executing
a sequence of instructions, as specified by the program, until it
terminates.  A multithreaded computation starts by executing a ***main
thread***, which is the thread at which the execution starts.  A thread
can create or ***spawn*** another thread and ***synchronize*** with other
threads by using a variety of synchronization constructs such as locks,
mutex's, synchronization variables, and semaphores.

DAG Representation
------------------

A multithreaded computation can be represented by a dag, a Directed
Acyclic Graph, or written also more simply a ***dag***, of vertices. The
figure below show an example multithreaded computation and its dag.
Each vertex represents the execution of an ***instruction***, such as an
addition, a multiplication, a memory operation, a (thread) spawn
operation, or a synchronization operation.  A vertex representing a
spawn operation has outdegree two.  A synchronization operation waits
for an operation belonging to a thread to complete, and thus a vertex
representing a synchronization operation has indegree two.  Recall
that a dag represents a partial order.  Thus the dag of the
computation represents the partial ordering of the dependencies
between the instructions in the computation.


![A multithreaded computation](images/general-threads.jpg)

Throughout this book, we make two assumptions about the structure of
the dag:

- Each vertex has outdegree at most two.

- The dag has exactly one ***root vertex*** with indegree zero and one
***final vertex*** vertex with outdegree zero.  The root is the first
instruction of the ***root thread***.

The outdegree assumption naturally follows by the fact that each
vertex represents an instruction, which can create at most one thread.

Cost Model: Work and Span
-------------------------

For analyzing the efficiency and performance of multithreaded
programs, we use several cost measures, the most important ones
include work and span.  We define the ***work*** of a computation as the
number of vertices in the dag and the ***span*** as the length of the
longest path in the dag.  In the example dag above, work is
$15$ and span is $9$.

Execution and Scheduling
------------------------

The execution of a multithreaded computation executes the vertices in the
dag of the computation in some partial order that is consistent with
the partial order specified by the dag, that is, if vertices
$u,v$ are ordered then the execution orders them in the
same way.  


Multithreaded programs are executed by using a ***scheduler*** that
assigns vertices of the dag to processes.  

::::: {#definition-execution-schedule .definition}

**Definition:** Execution Schedule

Given a dag $G$, an (execution) schedule for $G$ is a function from
processes and (time) ***steps*** to instructions of the dag such that

- if a vertex $u$ is ordered before another $v$ in $G$, then $v$ is
not executed at a time step before $u$, and

- each vertex in $G$  is executed exactly once.

The ***length*** of a schedule is the number of steps in the schedule.

:::::

The first condition ensures that a schedule observes the dependencies
in the dag.  Specifically, for each arc $(u,v)$ in the dag, the vertex
$u$ is executed before vertex $v$.

For any step in the execution, we call a vertex ***ready*** if all the
ancestors of the vertex in the dag are executed prior to that step.
Similarly, we say that a thread is ready if it contains a ready
vertex.  Note that a thread can contain only one ready vertex at any
time.

::::: {#ex-schedule .example}

**Example:** Schedule

Time Step      Process 1     Process 2     Process 3
---------      ---------     ---------     ---------
1              M1                          
2              M2                          
3              M3            A1                    
4                            A2            
5              B1            A3                     
6              B2            A4                      
7              B2                          M4         
8                            A5            M5         
9              A6                                   
10                           M6                     

Table: An example schedule with 3 processes. The length of this
schedule is $10$

:::::

::::: {#fact-scheduling-invariant .fact}

**Fact:** Scheduling Invariant

Consider a computation dag $G$ and consider an execution using any
scheduling algorithm.  At any time during the execution, color the
vertices that are executed as blue and the others as red.  

- The blue vertices induce a blue sub-dag of $G$ that is connected and
that has the same root as $G$.

- The red vertices incude a red sub-dag of $G$ that is connected.

- All the vertices of G are in the blue or the red sub-dag.  In other
words, the blue and red vertices partitions the dag into two sub-dags.

:::::

Scheduling Lower Bounds
-----------------------

::::: {#theorem-lower-bounds .theorem}

**Theorem:** Lower bounds

Consider any multithreaded computation with work $W$ and span $S$ and
$P$ processes.  The following lower bounds hold.

- Every execution schedule has length at least $\frac{W}{P}$.

- Every execution schedule has length at least $S$.

:::::

The first lower bound follows by the simple observation that a
schedule can only execute $P$ instructions at a time.  Since all
vertices must be executed, a schedule has length at least
$\frac{W}{P}$.  The second lower bound follows by the observation that
the schedule cannot execute a vertex before its ancestors and thus the
length of the schedule is at least as long as any path in the dag,
which can be as large as the span $S$.

Offline Scheduling
------------------

Having established a lower bound, we now move on to establish an upper
bound for the ***offline scheduling problem***, where we are given a
dag and wish to find an execution schedule that minimizes the run
time.  It is known that the related decision problem in NP-complete
but that 2-approximation is relatively easy.  We shall consider two
distinct schedulers: ***level-by-level scheduler*** and ***greedy
scheduler***.

A ***level-by-level schedule*** is a schedule that executes the
instructions in a given dag level order, where the ***level*** of a
vertex is the longest distance from the root of the dag to the vertex.
More specifically, the vertices in level 0 are executed first,
followed by the vertices in level 1 and so on.

::::: {#theorem-offline-level-by-level-schedule .theorem}

**Theorem:** Offline level-by-level schedule

For a dag with work $W$ and span $S$, the length of a level-by-level
schedule is $W/P + S$.

:::::

::::: {#proof-offline-level-by-level-schedule .proof}

**Proof**

Let $W_i$ denote the work of the instructions at level $i$.  These
instructions can be executed in $\lceil \frac{W_i}{P} \rceil$ steps.
Thus the total time is

$$
\sum_{i=1}^{S}{\lceil \frac{W_i}{P} \rceil} 
\le 
\sum_{i=1}^{S}{\lfloor \frac{W_i}{P} \rfloor + 1}
\le 
\lfloor \frac{W}{P} \rfloor + S
$$

:::::

This theorem, called [Brent's
theorem](https://www.google.com/search?q=Brent%27s+theorem&gws_rd=ssl),
was proved by Brent in 1974. It shows that the lower bound can be
approximated within a factor of $2$.

Brent's theorem has later been generalized to all greedy schedules.  A
***greedy schedule*** is a schedule that never leaves a process idle
unless there are no ready vertices.  In other words, greedy schedules
keep processes as busy as possibly by greedily assigning ready
vertices.

::::: {#theorem-greedy-schedule .theorem}

**Theorem:** Offline Greedy Schedule 

Consider a dag $G$ with work $W$ and span
$S$. Any greedy $P$-process schedule has
length at most $\frac{W}{P} + S \cdot \frac{P-1}{P}$.

:::::

::::: {#proof-greedy-schedule .proof}

**Proof**

Consider any greedy schedule with length $T$.

For each step $1 \le i \le T$, and for each process that
is scheduled at that step, collect a token.  The token goes to the
***work bucket*** if the process executes a vertex in that step,
otherwise the process is idle and the token goes to an ***idle bucket***.

Since each token in the work bucket corresponds to an executed vertex,
there are exactly $W$ tokens in that bucket.  

We will now bound the tokens in the idle bucket by $S \cdot
(P-1)$.  Observe that at any step in the execution schedule, there is
a ready vertex to be executed (because otherwise the execution is
complete).  This means that at each step, at most $P-1$
processes can contribute to the idle bucket.  Furthermore at each
step where there is at least one idle process, we know that the
number of ready vertices is less than the number of available
processes.  Note now that at that step, all the ready vertices have no
incoming edges in the red sub-dag consisting of the vertices that are
not yet executed, and all the vertices that have no incoming edges in
the red sub-dag are ready.  Thus executing all the ready vertices at
the step reduces the length of all the paths that originate at these
vertices and end at the final vertex by one. This means that the span
of the red sub-dag is reduced by one because all paths with length
equal to span must originate in a ready vertex. Since the red-subdag
is initially equal to the dag $G$, its span is
$S$, and thus there are at most $S$ steps at
which a process is idle.  As a result the total number of tokens in
the idle bucket is at most $S \cdot (P-1)$.

Since we collect $P$ tokens in each step, the bound thus
follows.

:::::

::::: {#ex-brent-bounds .exercise}

**Exercise**

Show that the bounds for Brent's level-by-level scheduler and for any
greedy scheduler is within a factor $2$ of optimal.

:::::

Online Scheduling
-----------------

In offline scheduling, we are given a dag and are interested in
finding a schedule with minimal length.  When executing multithreaded
program, however, we don't have full knowledge of the dag.  Instead,
the dag unfolds as we run the program.  Furthermore, we are interested
in not minimizing the length of the schedule but also the work and
time it takes to compute the schedule.  These two additional
conditions define the ***online scheduling problem.***


An online scheduler or a simply a ***scheduler*** is an algorithm that
solves the online scheduling problem by mapping threads to available
processes.  For example, if only one processor is available, a
scheduler can map all threads to that one processor.  If two
processors are available, then the scheduler can divide the threads
between the two processors as evenly as possible in an attempt to keep
the two processors as busy as possible by ***load balancing***.

There are many different online-scheduling algorithms but these
algorithms all operate similarly. We can outline a typical scheduling
algorithm as follows. 

::::: {#ex-typical-online-scheduling-algorithm .example}

**Example:** Typical Online Scheduling Algorithm

The algorithm maintains a ***work pool*** of work, consisting of ready
threads, and executes them. Execution starts with the root thread in
the pool.  It ends when the final vertex is executed.  In order to
minimize the cost of computing the schedule, the algorithm executes a
thread until there is a need for synchronization with other threads.

To obtain work, a process removes a thread from the pool and executes
its ready vertex. We refer to the thread executed by a process as the
***assigned thread***.  When executed, the ready vertex can make the
next vertex of the thread ready, which then also gets executed an so
on until one of the following ***synchronization*** actions occur.

- ***Die:*** The process executes last vertex of the thread, causing
the thread to die. The process then obtains other work.

- ***Block:*** The assigned vertex executes but the next vertex does
not become ready. This blocks the thread and thus the process obtains
other work.

- ***Enable:*** The assigned vertex makes ready the continuation of
the vertex and unblocks another previously blocked thread by making a
vertex from that thread ready. In this case, the process inserts both
(any) one thread into the work pool and continues to execute the
other.

- ***Spawn:*** The assigned vertex spaws another thread.  As in the
previous case, the process inserts one thread into the work pool and
continues to execute the other.

These actions are not mutually exclusive.  For example, a thread may
spawn/enable a thread and die.  In this case, the process performs the
corresponding steps for each action.
				   
:::::

::::: {#exercise-scheduling-invariant .exercise}

**Exercise:** Scheduling Invariant

Convince yourself that the scheduling invariant holds in online scheduling.

:::::

For a given schedule generated by an online scheduling algorithm, we
can define a tree of vertices, which tell us far a vertex, the vertex
that enabled it.

::::: {#definition-enabling-tree .definition}

**Definition:** Enabling Tree

Consider the execution of a dag.  If the execution of a vertex
$u$ enables another vertex $v$, then we call
the edge $(u,v)$ an ***enabling edge*** and we call
$u$ the ***enabling parent*** of $v$.  For
simplicity, we simply use the term ***parent*** instead of enabling
parent.

Note that any vertex other than the root vertex has one enabling
parent.  Thus the subgraph induced by the enabling edges is a rooted
tree that we call the ***enabling tree***.  

:::::

::::: {#ex-scheduler-with-global-thread-queue .example}

**Example:** Scheduler with a global thread queue.

We can give a simple greedy scheduler by using a queue of threads.  At
the start of the execution, the scheduler places the root thread into
the queue and then repeats the following step until the queue becomes
empty: for each idle process, take the thread at the front of the
queue and assign it to the processor, let each processor run for one
step, if at the end of the step, there are new ready threads, then
insert them onto the tail of the queue.

:::::

The centralized scheduler with the global thread queue is a greedy
scheduler that generates a greedy schedule under the assumption that
the queue operations take zero time and that the dag is given. This
algorithm, however, does not work well for online scheduling the
operations on the queue take time.  In fact, since the thread queue is
global, the algorithm can only insert and remove one thread at a time.
For this reason, centralized schedulers do not scale beyond a handful
of processors.

::::: {#definition-scheduling-friction .definition}

**Definition:** Scheduling friction

No matter how efficient a scheduler is there is real cost to creating
threads, inserting and deleting them from queues, and to performing
load balancing.  We refer to these costs cumulatively as ***scheduling
friction***, or simply as ***friction***.

:::::

There has been much research on the problem of reducing friction in
scheduling.  This research shows that distrubuted scheduling
algorithms can work quite well.  In a distributed algorithm, each
processor has its own queue and primarily operates on its own queue.
A load-balancing technique is then used to balance the load among the
existing processors by redistributing threads, usually on a needs
basis.  This strategy ensures that processors can operate in parallel
to obtain work from their queues.

A specific kind of distributed scheduling technique that can leads to
schedules that are close to optimal is ***work stealing***
schedulers. In a work-stealing scheduler, processors work on their own
queues as long as their is work in them, and if not, go "steal" work
from other processors by removing the thread at the tail end of the
queue.  It has been proven that randomized work-stealing algorithm,
where idle processors randomly select processors to steal from,
deliver close to optimal schedules in expectation (in fact with high
probability) and furthermore incur minimal friction.  Randomized
schedulers can also be implemented efficiently in practice. We
consider work-stealing in greater detail in future chapters.

Writing Multithreaded Programs: Pthreads
----------------------------------------

Multithreaded programs can be written using a variety of language
abstractions interfaces. One of the most widely used interfaces is the
***POSIX Threads*** or ***Pthreads*** interface, which specifies a
programming interface for a standardized C language in the IEEE POSIX
1003.1c standard. Pthreads provide a rich interface that enable the
programmer to create multiple threads of control that can synchronize
by using the nearly the whole range of the synchronization facilities
mentioned above.

::::: {#ex-pthread-hello-world .example}

**Example:** Hello world with Pthreads

An example Pthread program is shown below.  The main thread (executing
function `main`) creates 8 child threads and terminates.  Each child
in turn prints its "Hello world!" message and immediately
terminates. Since the main thread does not wait for the children to
terminate, it may terminate before the children does, depending on how
threads are scheduled on the available processors.

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ {.cpp}
#include <iostream>
#include <thread>

int main(int argc, char** argv) {
  static constexpr
  int nb_threads = 8;
  for (int i = 0; i < nb_threads; i++) {
    std::cout << "main: creating thread 00" << i << std::endl;
    auto t = std::thread([=] {
      std::cout << "Hello world! It is me, 00" << i << std::endl;
    });
    t.detach();
  }
  return 0;
}
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

When executed this program may print the following.

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
main: creating thread 000
main: creating thread 001
main: creating thread 002
main: creating thread 003
main: creating thread 004
main: creating thread 005
main: creating thread 006
main: creating thread 007
Hello world! It is me, 000
Hello world! It is me, 001
Hello world! It is me, 002
Hello world! It is me, 003
Hello world! It is me, 004
Hello world! It is me, 005
Hello world! It is me, 006
Hello world! It is me, 007
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

But that would be unlikely, a more likely output would look like this:

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
main: creating thread 000
main: creating thread 001
main: creating thread 002
main: creating thread 003
main: creating thread 004
main: creating thread 005
main: creating thread 006
main: creating thread 007
Hello world! It is me, 000
Hello world! It is me, 001
Hello world! It is me, 006
Hello world! It is me, 003
Hello world! It is me, 002
Hello world! It is me, 005
Hello world! It is me, 004
Hello world! It is me, 007
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

And may even look like this

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
main: creating thread 000
main: creating thread 001
main: creating thread 002
main: creating thread 003
Hello world! It is me, 000
Hello world! It is me, 001
Hello world! It is me, 003
Hello world! It is me, 002
main: creating thread 004
main: creating thread 005
main: creating thread 006
main: creating thread 007
Hello world! It is me, 006
Hello world! It is me, 005
Hello world! It is me, 004
Hello world! It is me, 007
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

:::::

Writing Multithreaded Programs: Structured or Implicit Multithreading
---------------------------------------------------------------------

Interface such as Pthreads enable the programmer to create a wide
variety of multithreaded computations that can be structured in many
different ways. Large classes of interesting multithreaded
computations, however, can be expcessed using a more structured
approach, where threads are restricted in the way that they
synchronize with other threads.  One such interesting class of
computations is fork-join computations where a thread can spawn or
"fork" another thread or "join" with another existing thread.  Joining
a thread is the only mechanism through which threads synchronize.  The
figure below illustrates a fork-join computation.  The main threads
forks thread A, which then spaws thread B.  Thread B then joins thread
A, which then joins Thread M.

![A multithreaded fork-join computation](images/fork-join-threads.jpg)

In addition to fork-join, there are other interfaces for structured
multithreading such as async-finish, and futures.  These interfaces
are adopted in many programming languages: the [Cilk
language](https://www.google.com/search?q=Cilk+language&gws_rd=ssl) is
primarily based on fork-join but also has some limited support for
async-finish; [X10
language](https://www.google.com/search?q=X10&gws_rd=ssl) is primarily
based on async-finish but also supports futures; the [Haskell
language](https://www.google.com/search?q=Haskell+language&gws_rd=ssl)
provides support for fork-join and futures as well as others;
[Parallel ML](https://www.google.com/search?q=Parallel+ML&gws_rd=ssl)
language as implemented by the [Manticore
project](http://manticore.cs.uchicago.edu/) is primarily based on
fork-join parallelism.  Such languages are sometimes called
***implicitly parallel***.

The class computations that can be expressed as fork-join and
async-finish programs are sometimes called ***nested parallel***.  The
term "nested" refers to the fact that a parallel computation can be
nested within another parallel computation.  This is as opposed to
***flat parallelism*** where a parallel computation can only perform
sequential computations in parallel.  Flat parallelism used to be
common technique in the past but becoming increasingly less prominent.

Parallelism versus concurrency
------------------------------
	
Structured multithreading offers important benefits both in terms of
efficiency and expressiveness.  Using programming constructs such as
fork-join and futures, it is usually possible to write parallel
programs such that the program accepts a "sequential semantics" but
executes in parallel.  The sequential semantics enables the programmer
to treat the program as a serial program for the purposes of
correctness.  A run-time system then creates threads as necessary to
execute the program in parallel.  This approach offers is some ways
the best of both worlds: the programmer can reason about correctness
sequentially but the program executes in parallel.  The benefit of
structured multithreading in terms of efficiency stems from the fact
that threads are restricted in the way that they communicate.  This
makes it possible to implement an efficient run-time system.

More precisely, consider some sequential language such as the untyped
(pure) lambda calculus and its sequential dynamic semantics specified as a
strict, small step transition relation.  We can extend this language
with the structured multithreading by enriching the syntax language
with "fork-join" and "futures" constructs.  We can now extend the
dynamic semantics of the language in two ways: 1) trivially ignore
these constructs and execute serially as usual, and 2) execute in
parallel by creating parallel threads.  We can then show that these
two semantics are in fact identical, i.e., that they produce the same
value for the same expressions.  In other words, we can extend a rich
programming language with fork-join and futures and still give the
language a sequential semantics.  This shows that structured
multithreading is nothing but an efficiency and performance concern;
it can be ignored from the perspective of correctness.

We use the term ***parallelism*** to refer to the idea of computing in
parallel by using such structured multithreading constructs.  As we
shall see, we can write parallel algorithms for many interesting
problems.  While parallel algorithms or applications constitute a
large class, they don't cover all applications.  Specifically
applications that can be expressed by using richer forms of
multithreading such as the one offered by Pthreads do not always
accept a sequential semantics. In such ***concurrent*** applications,
threads can communicate and coordinate in complex ways to accomplish
the intended result.  A classic concurrency example is the
"producer-consumer problem", where a consumer and a producer thread
coordinate by using a fixed size buffer of items.  The producer fills
the buffer with items and the consumer removes items from the buffer
and they coordinate to make sure that the buffer is never filled more
than it can take.  We can use operating-system level processes instead
of threads to implement similar concurrent applications.

In summary, parallelism is a property of the hardware or the software
platform where the computation takes place, whereas concurrency is a
property of the application.  Pure parallelism can be ignored for the
purposes of correctness; concurrency cannot be ignored for
understanding the behavior of the program.

Parallelism and concurrency are orthogonal dimensions in the space of
all applications.  Some applications are concurrent, some are not.
Many concurrent applications can benefit from parallelism.  For
example, a browser, which is a concurrent application itself as it may
use a parallel algorithm to perform certain tasks.  On the other hand,
there is often no need to add concurrency to a parallel application,
because this unnecessarily complicates software.  It can, however,
lead to improvements in efficiency.

The following quote from Dijkstra suggest pursuing the approach of
making parallelism just a matter of execution (not one of semantics),
which is the goal of the much of the work on the development of
programming languages today. Note that in this particular quote,
Dijkstra does not mention that parallel algorithm design requires
thinking carefully about parallelism, which is one aspect where
parallel and serial computations differ.

> From the past terms such as "sequential programming" and "parallel
> programming" are still with us, and we should try to get rid of
> them, for they are a great source of confusion.  They date from the
> period that it was the purpose of our programs to instruct our
> machines, now it is the purpose of the machines to execute our
> programs.  Whether the machine does so sequentially, one thing at a
> time, or with considerable amount of concurrency, is a matter of
> implementation, and should *not* be regarded as a property of the
> programming language.

quote, Edsger W. Dijkstra, Selected Writings on Computing: A Personal
Perspective (EWD 508)
