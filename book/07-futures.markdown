Structured Parallelism with Futures
===================================

Futures were first used for expressing parallelism in the context of
functional languages, because they permit a parallel computation to be
a first-class value. Such a value can be treated just like any other
value in the language. For example, it can be placed into a data
structure, passed to other functions as arguments. Another crucial
difference between futures and fork-join and async-finish parallelism
is synchronization: in the latter synchronization is guided by
"control dependencies", which are made apparent by the code. By
inspecting the code we can identify the synchronization points for a
piece of parallel computation, which correspond to join and finish. In
futures, synchronization can be more complex because it is based on
"data dependencies." When a parallel computation is needed, the
programmer can demand computation to be completed. If the computation
has not completed, then it will be executed to completion. If it has
completed, its result will be used. Using futures, we can parallelize
essentially any piece of computation, even if it depends on other
parallel computations. Recall that when using fork-join and
async-finish, we have had to ensure that the parallel computations
being spawned are indeed independent. This is not necessary with
futures.

To indicate a parallel computations, we shall use the `future`
construct, which takes an expression as an argument and starts a
parallel computation that will compute the value of that expression
sometime in the future. The construct returns a "future" value
representing the computation, which can be used just like any other
value of the language. When the time comes to use the value of the
future, we demand its completion by using the `force` construct.

Parallel Fibonacci via Futures
------------------------------

We can write a parallel version of Fibonacci using futures as follows.

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ {.cpp}
long fib_par(long n) {
  if (n < 2) {
    return n;
  } else {
    future<long> a, b;
    a = future([&] { return fib_par(n-1); });
    b = future([&] { return fib_par(n-2); });
    return force(a) + force(b);
  }
}
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

::::: {#note4 .note}

*Note:* This code is very similar to the fork-join version. In fact,
 we can directly map any use of fork-join parallelism to futures.

:::::

Much the same way that we represent fork-join and async-finish
computations with a dag of threads, we can also represent future-based
computations with a dag. The idea is to spawn a subdag for each future
and for each force add an edge from the terminus of that subdag to the
vertex that corresponds to the force. For example, for the Fibonacci
function, the dags with futures are identical to those with
fork-join. As we shall see, however, we can create more interesting
dags with futures.

Incrementing an array, in parallel
----------------------------------

We can write a parallel version of `map_incr` using futures by
following the same approach that we did for Fibonacci. But let’s
consider something slightly different. Suppose that the array is given
to us an array of future values. We can write a serial version of
`map_incr` for such an array as follows.

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ {.cpp}
void future_map_incr(future<int>* source, future<int>* dest, size_type n) {
  for (size_type i = 0; i < n; i++) {
    dest[i] = future([&] {
      return force(source[i]) + 1;
    });
  }
}
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

The function `future_map_incr` takes an array of futures of `int`s,
written `future<int>` and returns an array of the same type. To
compute each value of the array, the function `force`'s the
corresponding element of the source inside a future.

Futures and Pipelining
----------------------

Futures enable expressing parallelism at a fine level of granularity,
at the level of individual data dependencies. This ability enables
parallelizing computations at a similarly finer grain, enabling a
technique called pipelining.

The idea behind pipelining is to decompose a task $T$ into a sequence
of smaller subtasks $T_0,\ldots T_k$ to be performed in that order and
overlap the computation of multiple tasks by starting another instance
of the same kind of task as soon as the first subtask completes. As a
result, if we have a collection of the same kinds of tasks that can be
decomposed in the same way, we can have multiple of them "in flight"
without having to wait for one to complete.

When computing sequentially, pipelining does not help performance
because we can perform one computation at each time step. But when
using parallel hardware, we can keep multiple processors busy by
pipelining.

Suppose as an example, we have a sequence of tasks
$T^0,T^1,T^2,T^3,T^4,T^5$ that we wish to compute. Suppose furthermore
that these task depend on each other, that is a later task use the
results of an earlier task. Thus it might seem that there is no
parallelism that we can exploit. Upon further inspection, however, we
may realize that we can partition each task $T_i$ into subtasks
$T^i_0,T^i_1,T^i_2,T^i_3,T^i_4,T^i_5$ such that the subtask $j$ of
task $i$ is used by the subtask $j+1$ of task $i+1$. We can then
execute these tasks in parallel as shown below. Thus in the "steady
state" where we have a large supply of these tasks, we can increase
performance by a factor 5, the number of subtasks that a task
decomposes.

Processor	Step 0	Step 1	Step 2	Step 3	Step 4	Step 5
--------- ------  ------  ------  ------  ------  ------
$P_0$     $T^0_0$ $T^0_1$ $T^0_2$ $T^0_3$ $T^0_4$ $T^0_5$
$P_1$             $T^1_0$ $T^1_1$ $T^1_2$ $T^1_3$ $T^1_4$
$P_2$                     $T^2_0$ $T^2_1$ $T^2_2$ $T^2_3$
$P_3$                             $T^3_0$ $T^3_1$ $T^3_2$
$P_4$                                     $T^4_0$ $T^4_1$
$P_5$                                             $T^5_0$

This idea of pipelining turns out to be quite important in some
algorithms, leading sometimes to asymptotic improvements in
run-time. It can, however, be painful to design the algorithm to take
advantage of pipelining, especially if all we have available at our
disposal are fork-join and async-finish parallelism, which require
parallel computations to be independent. When using these constructs,
we might therefore have to redesign the algorithm so that independent
computations can be structurally separated and spawned in parallel. On
the other hand, futures make it trivial to express pipelined
algorithms because we can express data dependencies and ignore how
exactly the individual computations may need to be executed in
parallel. For example, in our hypothetical example, all we have to do
is create a future for each sub-task, and force the relevant subtask
as needed, leaving it to the scheduler to take advantage of the
parallelism made available.

As a more concrete example, let’s go back to our array increment
function and generalize to array map, and assume that we have another
function mk_array that populates the contents of the array.

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ {.cpp}
int f (int i) {
  ...
}

int g (int i) {
  ...
}

void mk_array (future<int>* source, size_type n) {
  for (size_type i = 0; i < n; i++) {
    source[i] = future([&] { return f(i); });
  }
}

void future_map_g (future<int>* source, future<int>* dest, size_type n) {
  for (size_type i = 0; i < n; i++) {
    dest[i] = future([&] {
      return g(force(source[i])); }
    );
  }
}

void doit(size_type n) {
  future<int>* source = new future<int>[n];
  future<int>* dest = new future<int>[n];
  mk_array(source, n);
  future_map_g(source, dest, n);
}
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

In this example, the $i^{th}$ element of the destination array depends
only on the $i^{th}$ element of the source, thus as soon as that
element is available, the function g might be invoked. This allows us
to pipeline the execution of the functions `mk_array` and
`future_map_g`.

While we shall not discuss this in detail, it is possible to improve
the asymptotic complexity of certain algorithms by using futures and
pipelining.

An important research question regarding futures is their
scheduling. One challenge is primarily contention. When using futures,
it is easy to create dag vertices, whose out-degree is
non-constant. The question is how can such dag nodes can be
represented to make sure that they don’t create a bottleneck, while
also allowing the small-degree instance, which is the common case, to
proceed efficiently. Another challenge is data locality. Async-finish
and fork-join programs can be scheduled to exhibit good data locality,
for example, using work stealing. With futures, this is more
tricky. It can be shown for example, that even a single scheduling
action can cause a large negative impact on the data locality of the
computation.
