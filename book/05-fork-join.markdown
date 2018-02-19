Fork-join parallelism
=====================

Fork-join parallelism, a fundamental model in parallel computing,
dates back to 1963 and has since been widely used in parallel
computing. In fork join parallelism, computations create opportunities
for parallelism by branching at certain points that are specified by
annotations in the program text.

Each branching point *forks* the control flow of the computation into
two or more logical threads. When control reaches the branching point,
the branches start running. When all branches complete, the control
*joins* back to unify the flows from the branches. Results computed by
the branches are typically read from memory and merged at the join
point. Parallel regions can fork and join recursively in the same
manner that divide and conquer programs split and join recursively. In
this sense, fork join is the divide and conquer of parallel computing.

As we will see, it is often possible to extend an existing language
with support for fork-join parallelism by providing libraries or
compiler extensions that support a few simple primitives.  Such
extensions to a language make it easy to derive a sequential program
from a parallel program by syntactically substituting the parallelism
annotations with corresponding serial annotations.  This in turn
enables reasoning about the semantics or the meaning of parallel
programs by essentially "ignoring" parallelism.

PASL is a C++ library that enables writing implicitly
parallel programs.  In PASL, fork join is expressed by application of
the `fork2()` function. The function expects two arguments: one for
each of the two branches. Each branch is specified by one
C++ lambda expression.

::::: {#ex-fork-join .example}

**Example 1:** Fork join

In the sample code below, the first branch writes the value 1 into the
cell `b1` and the second 2 into `b2`; at the join point, the sum of
the contents of `b1` and `b2` is written into the cell `j`.

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ {.cpp}
int b1 = 0;
int b2 = 0;
int j  = 0;

fork2([&] {
  // first branch
  b1 = 1;
}, [&] {
  // second branch
  b2 = 2;
});
// join point
j = b1 + b2;

std::cout << "b1 = " << b1 << "; b2 = " << b2 << "; ";
std::cout << "j = " << j << ";" << std::endl;
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Output:

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
b1 = 1; b2 = 2; j = 3;
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

When this code runs, the two branches of the fork join are both run to
completion. The branches may or may not run in parallel (i.e., on
different cores). In general, the choice of whether or not any two
such branches are run in parallel is chosen by the PASL runtime
system. The join point is scheduled to run by the PASL runtime only
after both branches complete. Before both branches complete, the join
point is effectively blocked. Later, we will explain in some more
detail the scheduling algorithms that the PASL uses to handle such
load balancing and synchronization duties.

:::::

In fork-join programs, a thread is a sequence of instructions that do
not contain calls to `fork2()`.  A thread is essentially a piece of
sequential computation.  The two branches passed to `fork2()` in the
example above correspond, for example, to two independent
threads. Moreover, the statement following the join point (i.e., the
continuation) is also a thread.

::::: {#note1 .note}

*Note:* If the syntax in the code above is unfamiliar, it might be a
good idea to review the notes on lambda expressions in C++11.  In a
nutshell, the two branches of `fork2()` are provided as
lambda-expressions where all free variables are passed by reference.

:::::

::::: {#note2 .note}

*Note:* Fork join of arbitrary arity is readily derived by repeated
application of binary fork join. As such, binary fork join is
universal because it is powerful enough to generalize to fork join of
arbitrary arity.

:::::

All writes performed by the branches of the binary fork join are
guaranteed by the PASL runtime to commit all of the changes that they
make to memory before the join statement runs. In terms of our code
snippet, all writes performed by two branches of `fork2` are committed
to memory before the join point is scheduled. The PASL runtime
guarantees this property by using a local barrier. Such barriers are
efficient, because they involve just a single dynamic synchronization
point between at most two processors.

::::: {#ex-writes-and-join .example}

**Example:** Writes and the join statement

In the example below, both writes into `b1` and `b2` are guaranteed to
be performed before the print statement.

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ {.cpp}
int b1 = 0;
int b2 = 0;

fork2([&] {
  b1 = 1;
}, [&] {
  b2 = 2;
});

std::cout << "b1 = " << b1 << "; b2 = " << b2 << std::endl;
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Output:

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
b1 = 1; b2 = 2
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Our fork-join library provides no guarantee on the visibility of
writes between any two parallel branches. In the code just above, for
example, writes performed by the first branch (e.g., the write to
`b1`) may or may not be visible to the second, and vice versa.

:::::

Parallel Fibonacci
------------------

Now, we have all the tools we need to describe our first parallel
code: the recursive Fibonacci function. Although useless as a program
because of efficiency issues, this example is the "hello world" program
of parallel computing.

Recall that the $n^{th}$ Fibonnacci number is defined
by the recurrence relation

$$
\begin{array}{lcl}
F(n) & = & F(n-1) + F(n-2)
\end{array}
$$

with base cases

$$
F(0) = 0, \, F(1) = 1
$$

Let us start by considering a sequential algorithm. Following the
definition of Fibonacci numbers, we can write the code for
(inefficiently) computing the $n^{th}$ Fibonnacci number
as follows. This function for computing the Fibonacci numbers is
inefficient because the algorithm takes exponential time, whereas
there exist dynamic programming solutions that take linear time.

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ {.cpp}
long fib_seq (long n) {
  long result;
  if (n < 2) {
    result = n;
  } else {
    long a, b;
    a = fib_seq(n-1);
    b = fib_seq(n-2);
    result = a + b;
  }
  return result;
}
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

To write a parallel version, we remark that the two recursive calls
are completely *independent*: they do not depend on each other
(neither uses a piece of data generated or written by another).  We
can therefore perform the recursive calls in parallel.  In general,
any two independent functions can be run in parallel.  To indicate
that two functions can be run in parallel, we use `fork2()`.

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ {.cpp}
long fib_par(long n) {
  long result;
  if (n < 2) {
    result = n;
  } else {
    long a, b;
    fork2([&] {
      a = fib_par(n-1);
    }, [&] {
      b = fib_par(n-2);
    });
    result = a + b;
  }
  return result;
}
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~


Incrementing an array, in parallel
----------------------------------

Suppose that we wish to map an array to another by incrementing each
element by one.  We can write the code for a function `map_incr` that
performs this computation serially.

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ {.cpp}
void map_incr(const int* source, int* dest, size_type n) {
  for (size_type i = 0; i < n; i++)
    dest[i] = source[i] + 1;
}
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

::::: {#ex-using-map-incr .example}

**Example:** Using `map_incr`

The code below illustrates  an example use of `map_incr`.

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ {.cpp}
const long n = 4;
int xs[n] = { 1, 2, 3, 4 };
int ys[n];
map_incr(xs, ys, n);
for (long i = 0; i < n; i++)
  std::cout << ys[i] << " ";
std::cout << std::endl;
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Output:

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
2 3 4 5 
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

:::::

Of course, the algorithm above is sequential. The following solution
represents a simple parallel version.

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ {.cpp}
void map_incr(const int* source, int* dest, size_type n) {
  map_incr_rec(source, dest, 0, n);
}

void map_incr_rec(const int* source, int* dest,
                  size_type lo, size_type hi) {
  size_type n = hi - lo;
  if (n == 0) {
    // do nothing
  } else if (n == 1) {
    dest[lo] = source[lo] + 1;
  } else {
    size_type mid = (lo + hi) / 2;
    fork2([&] {
      map_incr_rec(source, dest, lo, mid);
    }, [&] {
      map_incr_rec(source, dest, mid, hi);
    });
  }
}
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

It is easy to see that this algorithm has $O(n)$ work and $O(\log{n})$
span.

The sequential elision
----------------------

In the Fibonacci example, we started with a sequential algorithm and
derived a parallel algorithm by annotating independent functions.  It
is also possible to go the other way and derive a sequential algorithm
from a parallel one.  As you have probably guessed this direction is
easier, because all we have to do is remove the calls to the `fork2`
function. The sequential elision of our parallel Fibonacci code can be
written by replacing the call to `fork2()` with a statement that
performs the two calls (arguments of `fork2()`) sequentially as
follows.

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ {.cpp}
long fib_par(long n) {
  long result;
  if (n < 2) {
    result = n;
  } else {
    long a, b;
    ([&] {
      a = fib_par(n-1);
    })();
    ([&] {
      b = fib_par(n-2);
    })();
    result = a + b;
  }
  return result;
}
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

::::: {#note4 .note}

*Note:* Although this code is slightly different than the sequential
version that we wrote, it is not too far away, because the only the
difference is the creation and application of the lambda-expressions.
An optimizing compiler for C++ can easily "inline" such
computations. Indeed, After an optimizing compiler applies certain
optimizations, the performance of this code the same as the
performance of `fib_seq`.

:::::

The sequential elision is often useful for debugging and for
optimization.  It is useful for debugging because it is usually easier
to find bugs in sequential runs of parallel code than in parallel runs
of the same code.  It is useful in optimization because the
sequentialized code helps us to isolate the purely algorithmic
overheads that are introduced by parallelism. By isolating these
costs, we can more effectively pinpoint inefficiencies in our code.

::::: {#ex-fork-join-programs .example}

**Example:** Executing fork-join algorithms

We defined fork-join programs as a subclass case of multithreaded
programs.  Let's see more precisely how we can "map" a fork-join
program to a multithreaded program. An our running example, let's use the
`map_incr_rec`, whose code is reproduced below.

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ {.cpp}
void map_incr_rec(const int* source, int* dest,
                  size_type lo, size_type hi) {
  size_type n = hi - lo;
  if (n == 0) {
    // do nothing
  } else if (n == 1) {
    dest[lo] = source[lo] + 1;
  } else {
    size_type mid = (lo + hi) / 2;
    fork2([&] {
      map_incr_rec(source, dest, lo, mid);
    }, [&] {
      map_incr_rec(source, dest, mid, hi);
    });
  }
}
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

:::::

Since, a fork-join program does not explicitly manipulate threads, it
is not immediately clear what a thread refers to.  To define threads,
we can partition a fork-join computation into pieces of serial
computations, each of which constitutes a thread.  What we mean by a
serial computation is a computation that runs serially and also that
does not involve any synchronization with other threads except at the
start and at the end.  More specifically, for fork-join programs, we
can define a piece of serial computation a *thread*, if it executes
without performing parallel operations (`fork2`) except perhaps as its
last action.  When partitioning the computation into threads, it is
important for threads to be maximal; technically a thread can be as
small as a single instruction.

::::: {#definition-thread .definition}

**Definition:** Thread

A *thread* is a maximal computation consisting of a sequence of
instructions that do not contain calls to `fork2()` except perhaps at
the very end.

:::::

::::: {#ex-dag-parallel-increment .example}

**Example:** Dag for parallel increment on an array of $8$

![Each vertex corresponds a call to `map_inc_rec` excluding the
`fork2` or the continuation of `fork2`, which is empty, an is
annotated with the interval of the input array that it operates on
(its argument).](images/inc-parallel-dag.jpg)

:::::

The example above illustrates the dag for an execution of
`map_incr_rec`.  We partition each invocation of this function into
two threads labeled by "M" and "C" respectively.  The threads labeled
by $M\lbrack i,j \rbrack$ corresponds to the part of the invocation
of `map_incr_rec` with arguments `lo` and `hi` set to $i$ and $j$
respectively; this first part corresponds to the part of execution up
and including the `fork2` or all of the function if this is a base
case.  The second corresponds to the "continuation" of the `fork2`,
which is in this case includes no computation.

Based on this dag, we can create another dag, where each thread is
replaced by the sequence of instructions that it represents.  This
would give us a picture similar to the dag we drew before for general
multithreaded programs.  Such a dag representation, where we represent
each instruction by a vertex, gives us a direct way to calculate the
work and span of the computation.  If we want to calculate work and
span ond the dag of threads, we can label each vertex with a weight
that corresponds to the number of instruction in that thread.

::::: {#note5 .note}

*Note:* The term thread is very much overused in computer
science. There are *system threads*, which are threads that are known
to the operating system and which can perform a variety of
operations. For example, Pthreads library enables creating such system
threads and programming with them.  There are also many libraries for
programming with *user-level threads*, which are threads that exist at
the application level but the operating system does not know about
them.  Then there are threads that are much more specific such as
those that we have defined for the fork-join programs.  Such threads
can be mapped to system or user-level threads but since they are more
specific, they are usually implemented in a custom fashion, usually in
the user/application space. For this reason, some authors prefer to
use a different term for such threads, e.g., *spark*, *strand*,
*task*.

:::::

Let's observe a few properties of fork-join computations and their
dags.

- The computation dag of a fork-join program applied to an input
  unfolds dynamically as the program executes. For example, when we
  run `map_inc_rec` with an input with $n$ elements, the dag initially
  contains just the root vertex (thread) corresponding to the first
  call to `map_inc_rec` but it grows as the execution proceeds.
- An execution of a fork-join program can generate a massive number of
  threads.  For example, our `map_inc_rec' function generates
  approximately $4n$ threads for an input with $n$ elements.
- The work/span of each thread can vary from a small amount to a very
  large amount depending on the algorithm.  In our example, each
  thread performs either a conditional, sometimes an addition and a
  fork operation or performs no actual computation (continuation
  threads).

Suppose now we are given a computation dag and we wish to execute the
dag by mapping each thread to one of the $P$ processor that is
available on the hardware. To do this, we can use an online scheduling
algorithm.

::::: {#ex-two-proc-schedule .example}

**Example:** An example 2-processor schedule

The following is a schedule for the dag shown in the dab above
assuming that each thread takes unit time.

Time Step  Processor 1                Processor 2
---------  -----------                -----------
1          M [0,8)                    
2          M [0,4)                    M [4,8)
3          M [0,2)                    M [4,6)
4          M [0,1)                    M [4,5)
5          M [1,2)                    M [5,6)
6          C [0,2)                    C [4,6)
7          M [2,4)                    M [6,8)
8          M [2,3)                    M [6,7)
9          M [3,4)                    M [7,8)
10         C [2,4)                    C [6,8)
11         C [0,4)                    C [4,8)
12         C [0,8)                    _

Table: A 2-processor schedule.

:::::

::::: {#exercise-enabling-tree .exercise}

**Exercise:** Enabling Tree

Draw the enabling tree for the schedule above.

:::::

::::: {#ex-centralized-scheduler .example}

**Example:** Centralized scheduler

![Centralized scheduler illustrated: The state of the queue and the
dag after step 4.  Completed vertices are drawn in grey
(shaded).](images/centralized-scheduler.jpg)

:::::

::::: {#ex-distributed-scheduler .example}

**Example:** Distributed scheduler

![Distributed scheduler illustrated: The state of the queue and the
dag after step 4.  Completed vertices are drawn in grey
(shaded).](images/distributed-scheduler.jpg)

:::::
