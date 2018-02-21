Work efficiency
===============

In many cases, a parallel algorithm which solves a given problem
performs more work than the fastest sequential algorithm that solves
the same problem. This extra work deserves careful consideration for
several reasons.  First, since it performs additional work with
respect to the serial algorithm, a parallel algorithm will generally
require more resources such as time and energy.  By using more
processors, it may be possible to reduce the time penalty, but only by
using more hardware resources.

For example, if an algorithm performs $O(\log{n})$-factor more work
than the serial algorithm, then, assuming that the constant factor
hidden by the asymptotic notation is $1$, when $n = 2^{20}$, it will
perform $20$-times more actual work, consuming $20$ times more energy
consumption than the serial algorithm.  Assuming perfect scaling, we
can reduce the time penalty by using more processors.  For example,
with $40$ processors, the algorithm may require half the time of the
serial algorithm.

Sometimes, a parallel algorithm has the same asymptotic complexity of
the best serial algorithm for the problem but it has larger constant
factors.  This is generally true because scheduling friction,
especially the cost of creating threads, can be significant.  In
addition to friction, parallel algorithms can incur more communication
overhead than serial algorithms because data and processors may be
placed far away in hardware. For example, it is not unusual for a
parallel algorithm to incur a $10-100\times$ overhead over a similar
serial algorithm because of scheduling friction and communication.

These considerations motivate considering "work efficiency" of
parallel algorithm.  Work efficiency is a measure of the extra work
performed by the parallel algorithm with respect to the serial
algorithm.  We define two types of work efficiency: ***asymptotic work
efficiency*** and ***observed work efficiency***. The former relates to
the asymptotic performance of a parallel algorithm relative to the
fastest sequential algorithm. The latter relates to running time of a
parallel algorithm relative to that of the fastest sequential
algorithm.

::::: {#definition-asymptotic-work-efficiency .definition}

**Definition:** Asymptotic work efficiency

An algorithm is ***asymptotically work efficient*** if the work of the
algorithm is the same as the work of the best known serial algorithm.

:::::

::::: {#ex-asymptotic-work-efficiency .example}

**Example:** Asymptotic work efficiency

- A parallel algorithm that comparison-sorts $n$ keys in span
  $O(\log^3 n)$ and work $O(n \log n)$ is asymptotically work
  efficient because the work cost is as fast as the best known
  sequential comparison-based sorting algorithm. However, a parallel
  sorting algorithm that takes $O(n\log^2{n})$ work is not
  asymptotically work efficient.

- The parallel array increment algorithm that we consider in [an
  earlier Chapter](#chapter-fork-join) is asymptotically work
  efficient, because it performs linear work, which is optimal (any
  sequential algorithm must perform at least linear work).

:::::

To assess the practical effectiveness of a parallel algorithm, we
define observed work efficiency, parameterized a value
$r$.

::::: {#definition-observed-work-efficiency .definition}

**Definition:** Observed work efficiency

A parallel algorithm that runs in time $T_1$ on a single
processor has *observed work efficient factor of $r$* if
$r = \frac{T_1}{T_{seq}}$, where $T_{seq}$ is
the time taken by the fastest known sequential algorithm.

:::::

::::: {#ex-observed-work-efficiency .example}

**Example:** Observed work efficiency

- A parallel algorithm that runs $10\times$ slower on a single
  processor that the fastest sequential algorithm has an observed work
  efficiency factor of $10$.  We consider such algorithms
  unacceptable, as they are too slow and wasteful.

- A parallel algorithm that runs $1.1\times-1.2\times$. slower on a
  single processor that the fastest sequential algorithm has observed
  work efficiency factor of $1.2$.  We consider such algorithms to be
  acceptable.

:::::

::::: {#ex-observed-work-efficiency-increment .example}

**Example:** Observed work efficiency of parallel increment

To obtain this measure, we first run the baseline version of our
parallel-increment algorithm.

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
$ bench.baseline -bench map_incr -n 100000000
exectime 0.884
utilization 1.0000
result 2
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

We then run the parallel algorithm, which is the same exact code as
`map_incr_rec`. We build this code by using the special `optfp` "force
parallel" file extension. This special file extension forces
parallelism to be exposed all the way down to the base cases. Later,
we will see how to use this special binary mode for other purposes.

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
$ make bench.optfp
$ bench.optfp -bench map_incr -n 100000000
exectime 45.967
utilization 1.0000
result 2
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Our algorithm has an observed work efficiency factor of $60\times$.
Such poor observed work efficiency suggests that the parallel
algorithm would require more than an order of magnitude more energy
and that it would not run faster than the serial algorithm even when
using less than $60$ processors.

:::::

In practice, observed work efficiency is a major concern. First, the
whole effort of parallel computing is wasted if parallel algorithms
consistently require more work than the best sequential algorithms.
In other words, in parallel computing, both asymptotic complexity and
constant factors matter.

Based on these discussions, we define a ***good parallel algorithm*** as
follows.

::::: {#definition-good-parallel-algorithm .definition}

**Definition:** good parallel algorithm

We say that a parallel algorithm is ***good*** if it has the following
three characteristics:

- it is asymptotically work efficient;
- it is observably work efficient;
- it has low span.

:::::

Improving work efficiency with granularity control
--------------------------------------------------

It is common for a parallel algorithm to be asymptotically and/or
observably work inefficient but it is often possible to improve work
efficiency by observing that work efficiency increases with
parallelism and can thus be controlled by limiting it.

For example, a parallel algorithm that performs linear work and has
logarithmic span leads to average parallelism in the orders of
thousands with the small input size of one million.  For such a small
problem size, we usually would not need to employ thousands of
processors.  It would be sufficient to limit the parallelism so as to
feed tens of processors and as a result reduce impact of excess
parallelism on work efficiency.

In many parallel algorithms such as the algorithms based on
divide-and-conquer, there is a simple way to achieve this goal: switch
from parallel to sequential algorithm when the problem size falls
below a certain threshold.  This technique is sometimes called
***coarsening*** or ***granularity control***.

But which code should we switch to: one idea is to simply switch to
the sequential elision, which we always have available in PASL.  If,
however, the parallel algorithm is asymptotically work inefficient,
this would be ineffective.  In such cases, we can specify a separate
sequential algorithm for small instances.

Optimizing the practical efficiency of a parallel algorithm by
controlling its parallelism is sometimes called ***optimization***,
sometimes it is called ***performance engineering***, and sometimes
***performance tuning*** or simply ***tuning***.  In the rest of this
document, we use the term "tuning."

::::: {#ex-tuning-parallel-array-incr .example}

**Example:** Tuning  the parallel array-increment function

We can apply coarsening to `map_inc_rec` code by switching to the
sequential algorithm when the input falls below an established
threshold.

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ {.cpp}
size_type threshold = ... // some number;

void map_incr_rec(const int* source, int* dest, size_type lo, size_type hi) {
  size_type n = hi - lo;
  if (n <= threshold) {
    for (size_type i = lo; i < hi; i++)
      dest[i] = source[i] + 1;
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

::::: {#note-seqalg1 .note}

*Note:* Even in sequential algorithms, it is not uncommon to revert to
a different algorithm for small instances of the problem.  For
example, it is well known that insertion sort is faster than other
sorting algorithms for very small inputs containing 30 keys or less.
Many optimize sorting algorithm therefore revert to insertion sort
when the input size falls within that range.

:::::

::::: {#ex-observed-work-efficiency-parallel-array-incr .example}

**Example:** Observed work efficiency of tuned array increment

As can be seen below, after some tuning, `map_incr_rec` program
becomes highly work efficient. In fact, there is barely a difference
between the serial and the parallel runs.  The tuning is actually done
automatically here by using an automatic-granularity-control technique
described in the section.

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
$ bench.baseline -bench map_incr -n 100000000
exectime 0.884
utilization 1.0000
result 2
$ bench.opt -bench map_incr -n 100000000
exectime 0.895
utilization 1.0000
result 2
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

In this case, we have $r = \frac{T_1}{T_{seq}} = \frac{0.895}{0.884} =
1.012$ observed work efficiency. Our parallel program on a single
processor is one percent slower than the sequential baseline. Such
work efficiency is excellent.

:::::

Determining the threshold
-------------------------

The basic idea behind coarsening or granularity control is to revert
to a fast serial algorithm when the input size falls below a certain
threshold.  To determine the optimal threshold, we can simply perform
a search by running the code with different threshold settings.

While this approach can help find the right threshold on the
particular machine that we performed the search, there is no guarantee
that the same threshold would work on another machine.  In fact, there
are examples in the literature that show that such optimizations are
not ***portable***, i.e., a piece of code optimized for a particular
architecture may behave poorly on another. 

In the general case, determining the right threshold is even more
difficult.  To see the difficulty consider a generic (polymorphic),
higher-order function such as `map` that takes a sequence and a
function and applies the function to the sequence.  The problem is
that the threshold depends both on the type of the elements of the
sequence and the function to be mapped over the sequence.  For
example, if each element itself is a sequence (the sequence is
nested), the threshold can be relatively small.  If, however, the
elements are integers, then the threshold will likely need to be
relatively large.  This makes it difficult to determine the threshold
because it depends on arguments that are unknown at compile time.
Essentially the same argument applies to the function being mapped
over the sequence: if the function is expensive, then the threshold
can be relatively small, but otherwise it will need to be relatively
large.

As we describe in the [granularity-control
chapter](#chapter-granularity-control), it is sometimes possible to
determine the threshold completely automatically.
