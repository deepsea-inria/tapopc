Automatic granularity control {#chapter-granularity-control}
=============================

There has been significant research into determining the right
threshold for a particular algorithm.  This problem, known as the
***granularity-control problem***, turns out to be a rather difficult
one, especially if we wish to find a technique that can ensure
close-to-optimal performance across different architectures.  In this
section, we present a technique for automatically controlling
granularity by using asymptotic cost functions.

Complexity functions
--------------------

Our automatic granularity-control technique requires assistance from
the application programmer: for each parallel region of the program,
the programmer must annotate the region with a *complexity function*,
which is simply a C++ function that returns the asymptotic work cost
associated with running the associated region of code.

::::: {#ex-complexity-function-map-incr .example}

**Example:** Complexity function of `map_incr_rec`

An application of our `map_incr_rec` function on a given range $[lo,
hi)$ of an array has work cost $cn = c (hi - lo)$ for some constant
$c$. As such, the following lambda expression is one valid complexity
function for our parallel `map_incr_rec` program.

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ {.cpp}
auto map_incr_rec_complexity_fct = [&] (size_type lo, size_type hi) {
  return hi - lo;
};
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

In general, the value returned by the complexity function need only be
precise with respect to the asymptotic complexity class of the
associated computation.  The following lambda expression is another
valid complexity function for function `map_incr_rec`.  The complexity
function above is preferable, however, because it is simpler.

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ {.cpp}
const size_type k = 123;
auto map_incr_rec_complexity_fct2 = [&] (size_type lo, size_type hi) {
  return k * (hi - lo);
};
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

:::::

More generally, suppose that we know that a given algorithm has work
cost of $W = n + \log n$. Although it would be fine to assign to this
algorithm exactly $W$, we could just as well assign to the algorithm
the cost $n$, because the second term is dominated by the first.  In
other words, when expressing work costs, we only need to be precise up
to the asymptotic complexity class of the work.

Controlled statements
---------------------

In SPTL, a ***controlled statement***, or `cstmt`, is an annotation in
the program text that activates automatic granularity control for a
specified region of code. In particular, a controlled statement
behaves as a C++ statement that has the special ability to choose on
the fly whether or not the computation rooted at the body of the
statement spawns parallel threads.  To support such automatic
granularity control SPTL uses a prediction algorithm to map the
asymptotic work cost (as returned by the complexity function) to
actual processor cycles.  When the predicted processor cycles of a
particular instance of the controlled statement falls below a
threshold (determined automatically for the specific machine), then
that instance is sequentialized, by turning off the ability to spawn
parallel threads for the execution of that instance.  If the predicted
processor cycle count is higher than the threshold, then the statement
instance is executed in parallel.

In other words, the reader can think of a controlled statement as a
statement that executes in parallel when the benefits of parallel
execution far outweigh its cost and that executes sequentially in a
way similar to the sequential elision of the body of the controlled
statement would if the cost of parallelism exceeds its benefits.  We
note that while the sequential exection is similar to a sequential
elision, it is not exactly the same, because every call to `fork2`
must check whether it should create parallel threads or run
sequentially.  Thus the execution may differ from the sequential
elision in terms of performance but not in terms of behavior or
semantics.

::::: {#ex-array-increment-auto-granularity .example}

**Example:** Array-increment function with automatic granularity control

The code below uses a controlled statement to automatically select, at
run time, the threshold size for our parallel array-increment
function.

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ {.cpp}
void map_incr_rec(const int* source, int* dest, size_type lo, size_type hi) {
  size_type n = hi - lo;
  spguard([&] { return n; }, [&] {
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
  });
}
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

:::::

The controlled statement takes three arguments, whose requirements are
specified below, and returns nothing (i.e., `void`). The effectiveness
of the granularity controller may be compromised if any of the
requirements are not met.

- The first argument is a reference to the controller object. The
controller object is used by the controlled statement to collect
profiling data from the program as the program runs. Every controller
object is initialized with a string label (in the code above
`"map_incr_rec"`).  The label must be unique to the particular
controller. Moreover, the controller must be declared as a global
variable.

- The second argument is the complexity function. The type of the
return value should be `size_type`.

- The third argument is the body of the controlled statement. The
return type of the controlled statement should be `void`.

When the controlled statement chooses sequential evaluation for its
body the effect is similar to the effect where in the code above the
input size falls below the threshold size: the body and the recursion
tree rooted there is sequentialized. When the controlled statement
chooses parallel evaluation, the calls to `fork2()` create parallel
threads.

It is not unusual for a divide-and-conquer algorithm to switch to a
different algorithm at the leaves of its recursion tree. For example,
sorting algorithms, such as quicksort, may switch to insertion sort at
small problem sizes. In the same way, it is not unusual for parallel
algorithms to switch to different sequential algorithms for handling
small problem sizes.  Such switching can be beneficial especially when
the parallel algorithm is not asymptotically work efficient.

To provide such algorithmic switching, SPTL provides an alternative
form of controlled statement that accepts a fourth argument: the
***alternative sequential body***.  This alternative form of
controlled statement behaves essentially the same way as the original
described above, with the exception that when SPTL run time decides to
sequentialize a particular instance of the controlled statement, it
falls through to the provided alternative sequential body instead of
the "sequential elision."

::::: {#ex-array-increment-alt-body .example}

**Example:** Array-increment function with automatic granularity control and sequential body

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ {.cpp}
void map_incr_rec(const int* source, int* dest, size_type lo, size_type hi) {
  size_type n = hi - lo;
  spguard([&] { return n; }, [&] {
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
  }, [&] {
    for (size_type i = lo; i < hi; i++)
      dest[i] = source[i] + 1;
  });
}
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Even though the parallel and sequential array-increment algorithms are
algorithmically identical, except for the calls to `fork2()`, there is
still an advantage to using the alternative sequential body: the
sequential code does not pay for the parallelism overheads due to
`fork2()`. Even when eliding `fork2()`, the run-time-system has to
perform a conditional branch to check whether or not the context of
the `fork2()` call is parallel or sequential. Because the cost of
these conditional branches adds up, the version with the sequential
body is going to be more work efficient.  Another reason for why a
sequential body may be more efficient is that it can be written more
simply, as for example using a for-loop instead of recursion, which
will be faster in practice.

:::::

::::: {#note-granularity-style .note}

*Note:* Recommended style for programming with controlled statements

In general, we recommend that the code of the parallel body be written
so as to be completely self contained, at least in the sense that the
parallel body code contains the logic that is necessary to handle
recursion all the way down to the base cases.  The code for
`map_incr_rec` honors this style by the fact that the parallel body
handles the cases where `n` is zero or one (base cases) or is greater
than one (recursive case). Put differently, it should be the case
that, if the parallelism-specific annotations (including the
alternative sequential body) are erased, the resulting program is a
correct program.

We recommend this style because such parallel codes can be debugged,
verified, and tuned, in isolation, without relying on alternative
sequential codes.

:::::

Controlled parallel-for loops
-----------------------------

Let us add one more component to our granularity-control toolkit: the
***parallel-for*** from. By using this loop construct, we can avoid
having to explicitly express recursion-trees over and over again. For
example, the following function performs the same computation as the
example function we defined in the first lecture. Only, this function
is much more compact and readable. Moreover, this code takes advantage
of our automatic granularity control, also by replacing the
parallel-for with a serial-for.

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ {.cpp}
void map_incr(const int* source, int* dest, size_type n) {
  parallel_for((size_type)0, n, [&] (size_type i) {
    dest[i] = source[i] + 1;
  });
}
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Underneath, the parallel-for loop uses a divide-and-conquer routine
whose structure is similar to the structure of the divide-and-conquer
routine of our `map_incr_rec`. Because the parallel-for loop generates
the log-height recursion tree, the `map_incr` routine just above has
the same span as the `map_incr` routine that we defined earlier: $\log
n$, where $n$ is the size of the input array.

Notice that the code above specifies no complexity function. The
reason is that this particular instance of the parallel-for loop
implicitly defines a complexity function. The implicit complexity
function reports a linear-time cost for any given range of the
iteration space of the loop. In other words, the implicit complexity
function assumes that per iteration the body of the loop performs a
constant amount of work. Of course, this assumption does not hold in
general. If we want to specify explicitly the complexity function, we
can use the form shown in the example below. The complexity function
is passed to the parallel-for loop as the fourth argument.  The
complexity function takes as argument the range `[lo, hi)`. In this
case, the complexity is linear in the number of iterations. The
function simply returns the number of iterations as the complexity.

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ {.cpp}
void map_incr(const int* source, int* dest, size_type n) {
  auto linear_complexity_fct = [] (size_type lo, size_type hi) {
    return hi - lo;
  };
  parallel_for(linear_complexity_fct, (size_type)0, n, [&] (size_type i) {
    dest[i] = source[i] + 1;
  });
}
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

The following code snippet shows a more interesting case for the
complexity function. In this case, we are performing a multiplication
of a dense matrix by a dense vector. The outer loop iterates over the
rows of the matrix. The complexity function in this case gives to each
of these row-wise iterations a cost in proportion to the number of
scalars in each column.

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ {.cpp}
// mtx: nxn dense matrix, vec: length n dense vector
// dest: length n dense vector
void dmdvmult(double* mtx, double* vec, double* dest, size_type n) {
  auto compl_fct = [&] (size_type lo, size_type hi) {
    return (hi - lo) * n;
  };
  parallel_for(compl_fct, (size_type)0, n, [&] (size_type i) {
    ddotprod(mtx, v, dest, i);
  });
  return dest;
}
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

::::: {#ex-speedup-matrix-multiply .example}

**Example:** Speedup for matrix multiply

Matrix multiplication has been widely used as an example for parallel
computing since the early days of the field.  There are good reasons
for this. First, matrix multiplication is a key operation that can be
used to solve many interesting problems. Second, it is an expansive
computation that is nearly cubic in the size of the input---it can
thus can become very expensive even with modest inputs.

Fortunately, matrix multiplication can be parallelized relatively
easily as shown above.  The figure below shows the speedup for a
sample run of this code.  Observe that the speedup is rather good,
achieving nearly excellent utilization.

![Speedup plot for matrix multiplication for $25000 \times 25000$
 matrices.](images/dmdvmult-speedup.jpg)

:::::

While parallel matrix multiplication delivers excellent speedups, this
is not common for many other algorithms on modern multicore machines
where many computations can quickly become limited by the availability
of bandwidth.  Matrix multiplication does not suffer as much from the
memory-bandwidth limitations because it performs significant work per
memory operation: it touches $\Theta(n^2)$ memory cells, while
performing nearly $\Theta(n^3)$ work.
