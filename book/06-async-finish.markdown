Structured Parallelism with Async-Finish
========================================

The "async-finish" approach offers another mechanism for structured
parallelism. It is similar to fork-join parallelism but is more
flexible, because it allows essentially any number of parallel
branches to synchronize at the same point called a "finish". Each
"async" creates a separate parallel branch, which by construction must
signal a "finish."

As with fork-join, it is often possible to extend an existing language
with support for async-finish parallelism by providing libraries or
compiler extensions that support a few simple primitives. Such
extensions to a language make it easy to derive a sequential program
from a parallel program by syntactically substituting the parallelism
annotations with corresponding serial annotations. This in turn
enables reasoning about the semantics or the meaning of parallel
programs by essentially "ignoring" parallelism, e.g., via the
sequential elision mechanism.

Parallel Fibonacci via Async-Finish
-----------------------------------

To write a parallel version, we remark that the two recursive calls
are completely independent: they do not depend on each other (neither
uses a piece of data generated or written by another). We can
therefore perform the recursive calls in parallel. In general, any two
independent functions can be run in parallel. To indicate that two
functions can be run in parallel, we use `async()`. It is a
requirement that each async takes place in the context of a `finish()`
block. All async'ed computations that takes place in the context of a
finish synchronize at that finish, i.e., finish completes only when
all the async'ed computations complete. The important point about a
finish is that it can serve as the synchronization point for an
arbitrary number of async'ed computations. This is not quite useful in
Fibonacci because there are only two computations to synchronize at a
time, but it will be useful in our next example.

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ {.cpp}
long fib_par(long n, long& result) {
  if (n < 2) {
    result = n;
  } else {
    long a, b;
    finish([&] {
      async([&] { fib_par(n-1, a); });
      aysnc([&] { fib_par(n-2, b); });
    });
    result = a + b;
  }
}
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Recall our example for mapping an array to another by incrementing
each element by one. We can write the code for a function map_incr
that performs this computation serially.

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ {.cpp}
void map_incr(const int* source, int* dest, size_type n) {
  for (size_type i = 0; i < n; i++)
    dest[i] = source[i] + 1;
}
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Using async-finish, we can write to code parallel array increment by
using only a single ‘finish` block and async’ing all the parallel
computations inside that finish.

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ {.cpp}
void map_incr(const int* source, int* dest, size_type n) {
  finish([&] { map_incr_rec(source, dest, 0, n)); };
}

void map_incr_rec(const int* source, int* dest, size_type lo, size_type hi) {
  size_type n = hi - lo;
  if (n == 0) {
    // do nothing
  } else if (n == 1) {
    dest[lo] = source[lo] + 1;
  } else {
    size_type mid = (lo + hi) / 2;
    async([&] { map_incr_rec(source, dest, lo, mid); };
    async([&] { map_incr_rec(source, dest, mid, hi); });
  }
}
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

It is helpful to compare this to fork-join, where we had to
synchronize parallel branches in pairs. Here, we are able to
synchronize them all at a single point.

![Dag for parallel increment on an array of 8 using async-finish: Each
 vertex corresponds a call to `map_inc_rec` excluding the async or the
 continuation of async, which is empty, and is annotated with the
 interval of the input array that it operates on (its
 argument).](images/inc-parallel-dag-async-finish.jpg){#img-async-finish}

The [Figure above](#img-async-finish) illustrates the dag for an
execution of `map_incr_rec`. Each invocation of this function
corresponds to a thread labeled by "M". The threads labeled by
$M[i,j]$ correspond to the part of the invocation of `map_incr_rec`
with arguments `lo` and `hi` set to $i$ and $j$ respectively. Note
that all threads synchronize at the single finish node.

Based on this dag, we can create another dag, where each thread is
replaced by the sequence of instructions that it represents. This new
dag would give us a picture similar to the dag we drew before for
general multithreaded programs. Such a dag representation, where we
represent each instruction by a vertex, gives us a direct way to
calculate the work and span of the computation. If we want to
calculate work and span on the dag of threads, we can label each
vertex with a weight that corresponds to the number of instruction in
that thread.

Using async-finish does not alter the asymptotic work and span of the
computation compared to fork-join, which remain as $O(n)$ work and
$O(\log n)$ span. In practice, however, the async-finish version
creates fewer threads (by about a factor of two), which can make a
difference.
