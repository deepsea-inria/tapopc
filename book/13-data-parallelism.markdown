Data parallelism
================

Tabulation
----------

A ***tabulation*** is a parallel operation which creates a new array
of a given size and initializes the contents according to a given
*generator function*. In SPTL, tabulation is made available by the
following constructor from the parallel array class.

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ {.cpp}
template <class Generator>
parray(size_type n, Generator g);
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

The application of the constructor to a number `n` and a generator
function `g` allocates an array of length `n` and assigns to each
index in the array `i` the value returned by `g(i)`.

::::: {#ex-tabulation-evens .example}

**Example:** Generating even numbers

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ {.cpp}
parray<int> evens(5, [&] (size_type i) {
  return 2 * i;
});
std::cout << "evens = " << evens << std::endl;
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Output:

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
evens = { 0, 2, 4, 6, 8 }
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

:::::

Copying an array can be expressed as a tabulation.

::::: {#ex-tabulation-copy .example}

**Example:** Copying an array

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ {.cpp}
parray<int> mycopy(const parray<int>& src) {
  parray<int> dst(src.size(), [&] (size_type i) {
    return src[i];
  });
}
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

:::::

::::: {#exercise-tabulate-dup-ktimes .exercise}

**Exercise:** Using tabulation

Solve the `duplicate` and `ktimes` problems that were given in
homework, this time using tabulations.

Solutions appear below.

:::::

::::: {#ex-tabulation-exercise-solutions .example}

**Example:** Solution for the tabulation exercise

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ {.cpp}
parray<int> ktimes(const parray<int>& src, size_type k) {
  size_type m = xs.size() * k;
  parray<int> dst(m, [&] (size_type i) {
    return src[i / k];
  });
  return dst;
}

parray<int> duplicate(const parray<int>& src) {
  return ktimes(src, 2);
}
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

:::::

Thus far, we have considered tabulations that use a generator function
with constant-time work complexity. Let us first analyze the work and
span for the simple case, in which the generator function takes
constant work, and then handle the general case.  In the simple case,
it should be clear that the tabulation should take work linear in the
size of the array. The reason is that the only work performed by the
body of the loop is performed by the constant-time generator
function. Since the loop itself performs as many iterations as
positions in the array, the work cost is indeed linear in the size of
the array. The span cost of the tabulation is the sum of two
quantitles: the span taken by the loop and the maximum value of the
spans taken by the applications of the generator function. Recall that
we saw before that the span cost of a parallel-for loop with $n$
iterations is $\log n$. The maximum of the spans of the generator
applications is a constant. Therefore, we can conclude that, in this
case, the span cost is logarithmic in the size of the array.

The story is only a little more complicated when we generalize to
consider non-constant time generator functions. Let $W(\mathtt{g}(i))$
denote the work performed by an application of the generator function
to a specified value $i$. Similarly, let $S(\mathtt{g}(i))$ denote the
span. Then the tabulation takes work

$$
\sum_{i=0}^{n} W(\mathtt{g}(i))
$$

and span

$$
\log n + \max_{i=0}^{n} S(\mathtt{g}(i))
$$

If the generator function has non-constant work complexity, then the
tabulation takes the form below, where the tabulation takes an
addtional argument to represent the complexity function of the
generator function.

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ {.cpp}
template <class Complexity, class Generator>
parray(size_type n, Complexity c, Generator g);
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

::::: {#ex-tabulation-complexity-function .example}

**Example:** Tabulation with complexity function

Here, we return to our [matrix-multiplication
example](#ex-dense-matrix-multiply). This time, however, we use a
tabulation to materialize the result vector `r`, whereas in the
original example we used destination passing style. Because the
application of the generator function, in this case, the dot product,
takes linear work, the complexity function `compl_fct` reports linear
complexity for each iteration of the tabulation.

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ {.cpp}
parray<double> dmdvmult2(double* mtx, double* vec, double* dest, size_type n) {
  auto compl_fct = [&] (size_type lo, size_type hi) {
    return (hi - lo) * n;
  };
  parray<double> r(xss.size(), compl_fct, [&] (size_type i) {
    return ddotprod(mtx, v, dest, i);
  });
  return r;
}
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

:::::

Reduction
---------

A ***reduction*** is an operation which combines a given set of values
according to a specified *identity element* and a specified
*associative combining operator*. Let $S$ denote a set. Recall from
algebra that an associative combining operator is any binary operator
$\oplus$ such that, for any three items $x,y,z \in S$, the following
holds.

$$
x \oplus (y \oplus z) = (x \oplus y) \oplus z
$$

An element $\mathbf{I} \in S$ is an identity element if for any $x \in
S$ the following holds.

$$
(x \oplus \mathbf{I}) \, = \, (\mathbf{I} \oplus x) \, = \, x
$$

This algebraic structure consisting of $(S, \oplus, \mathbf{I})$ is
called a ***monoid*** and is particularly worth knowing because this
structure is a common pattern in parallel computing.

::::: {#ex-reduction-addition-monoid .example}

**Example:** Addition monoid

- $S$ = the set of all 64-bit unsigned integers
- $\oplus$ = addition modulo $2^{64}$
- $\mathbf{I}$ = 0

:::::

::::: {#ex-reduction-multiplication-monoid .example}

**Example:** Multiplication monoid

- $S$ = the set of all 64-bit unsigned integers
- $\oplus$ = multiplication modulo $2^{64}$
- $\mathbf{I}$ = 1

:::::

::::: {#ex-reduction-max-monoid .example}

**Example:** Max monoid

- $S$ = the set of all 64-bit unsigned integers
- $\oplus$ = max function
- $\mathbf{I}$ = 0

:::::

The identity element is important because we are working with
sequences: having a base element is essential for dealing with empty
sequences. For example, what should the sum of the empty sequence?
More interestingly, what should be the maximum (or minimum) element of
an empty sequence? The identity element specifies this behavior.

What about the associativity of $\oplus$? Why does associativity
matter? Suppose we are given the sequence $[ a_0, a_1, \ldots, a_n
]$. The serial reduction of this sequence always computes the
expression $(a_0 \oplus a_1 \oplus \ldots \oplus a_n)$. However, when
the reduction is performed in parallel, the expression computed by the
reduction could be $( (a_0 \oplus a_1 \oplus a_2 \oplus a_3) \oplus
(a_4 \oplus a_5) \oplus \ldots \oplus (a_{n-1} \oplus a_n) )$ or $(
(a_0 \oplus a_1 \oplus a_2) \oplus (a_3 \oplus a_4 \oplus a_5) \oplus
\ldots \oplus (a_{n-1} \oplus a_n) )$. In general, the exact placement
of the parentheses in the parallel computation depends on the way that
the parallel algorithm decomposes the problem. Associativity gives the
parallel algorithm the flexibility to choose an efficient order of
evaluation and still get the same result in the end. The flexibility
to choose the decomposition of the problem is exploited by efficient
parallel algorithms, for reasons that should be clear by now. In
summary, associativity is a key building block to the solution of many
problems in parallel algorithms.

Now that we have monoids for describing a generic method for combining
two items, we can consider a generic method for combining many items
in parallel. Once we have this ability, we will see that we can solve
the remaining problems from last homework by simply plugging the
appropriate monoids into our generic operator, `reduce`. The interface
of this operator in our framework is specified below. The first
parameter corresponds to $\oplus$, the second to the identity element,
and the third to the sequence to be processed.

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ {.cpp}
template <class Iter, class Item, class Combine>
Item reduce(Iter lo, Iter hi, Item id, Combine combine);
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

We can solve our first problem by plugging integer plus as $\oplus$
and 0 as $\mathbf{I}$.

::::: {#ex-summing-array-by-reduction .example}

**Example:** Summing elements of array

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ {.cpp}
auto plus_fct = [&] (int x, int y) {
  return x+y;
};

parray<int> xs = { 1, 2, 3 };

std::cout << "reduce(xs.begin(), xs.end(), 0, plus_fct) = "
          << reduce(xs.begin(), xs.end(), 0, plus_fct)
          << std::endl;
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Output:

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
reduce(xs.begin(), xs.end(), 0, plus_fct) = 6
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

:::::

We can solve our second problem in a similar fashion. Note that in
this case, since we know that the input sequence is nonempty, we can
pass the first item of the sequence as the identity element.  What
could we do if we instead wanted a solution that can deal with
zero-length sequences? What identity element might make sense in that
case? Why?

::::: {#ex-array-max-by-reduction .example}

**Example:** Taking max of elements of array

Let us start by solving a special case: the one where the input
sequence is nonempty.

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ {.cpp}
auto max_fct = [&] (int x, int y) {
  return std::max(x, y);
};

parray<int> xs = { -3, 1, 634, 2, 3 };

std::cout << "reduce(xs.begin(), xs.end(), xs[0], max_fct) = "
          << reduce(xs.begin(), xs.end(), xs[0], max_fct)
          << std::endl;
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Output:

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
reduce(xs.begin(), xs.end(), xs[0], max_fct) = 634
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Observe that in order to seed the reduction we selected the
provisional maximum value to be the item at the first position of the
input sequence. Now let us handle the general case by seeding with the
smallest possible value of type `int`.

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ {.cpp}
int max(parray<int>& xs) {
  int id = std::numeric_limits<int>::lowest();
  return reduce(xs.begin(), xs.end(), id, [&] (int x, int y) {
    return std::max(x, y);
  });
}
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

The value of `std::numeric_limits<int>::lowest()` is defined by the
header file `<limits>`.

:::::

Like the tabulate function, reduce is a higher-order function. Just
like any other higher-order function, the work and span costs have to
account for the cost of the client-supplied function, which is in this
case, the associative combining operator.

There are two cases to consider for any reduction $\mathtt{reduce}(lo,
hi, id, f)$:

1. ***Constant-time associative combining operator.*** The amount of
work performed by the reduction is $O(hi-lo)$ and the span is $O(\log
(hi-lo))$.

2. ***Non-constant-time associative combining operator.*** We define
$\mathcal{R}$ to be the set of all function applications $f(x, y)$
that are performed in the reduction tree. Then,

    - The work performed by the reduction is $O(n + \sum_{f(x, y) \in
    \mathcal{R}(f, id, lo, hi)} W(f(x, y)))$.

    - The span of the reduction is $O(\log n \max_{f(x, y) \in
    \mathcal{R}(f, id, lo, hi)} S(f(x, y)))$.

Under certain conditions, we can use the following lemma to deduce a
more precise bound on the amount of work performed by the
reduction.

::::: {#lemma-work-efficiency .lemma}

***Lemma (Work efficiency).*** For any associative combining operator
$f$ and weight function $w$, if for any $x$, $y$,

- $w(f(x, y)) \leq w(x) + w(y)$, and
- $W \leq c (w(x) + w(y))$, for some constant $c$,

where $W$ denotes the amount of work performed by the call $f(x, y)$,
then the amount of work performed by the reduction is $O(\log (hi-lo)
\sum_{lo \leq it < hi} (1 + w(*it)))$.

:::::

::::: {#todo-reduction-examples .todo}

**TODO:** Give some code examples and analyses of programs with
interesting non-constant-time reductions.

:::::

Scan
----

A ***scan*** is an iterated reduction that comes in four
varieties. Because each of the two dimensions in the `scan_type` are
symmetrical, the concept is easy to see if we consider just, say,
`forward_inclusive_scan`, for example.

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ {.cpp}
using scan_type = enum {
  forward_inclusive_scan,
  forward_exclusive_scan,
  backward_inclusive_scan,
  backward_exclusive_scan
};
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

::::: {#ex-forward-inclusive-scan .example}

**Example:** Forward-inclusive scan

The inclusive form maps a given sequence $[ x_0, x_1, x_2, \ldots,
x_{n-1} ]$ to $[ x_0, x_0 \oplus x_1, x_0 \oplus x_1 \oplus x_2,
\ldots, x_0 \oplus x_1 \oplus \ldots \oplus x_{n-1} ]$.

:::::

::::: {#ex-forward-exclusive-scan .example}

**Example:** Forward-exclusive scan

The exclusive form maps a given sequence $[ x_0, x_1, x_2, \ldots,
x_{n-1} ]$ to $[ \mathbf{I}, x_0, x_0 \oplus x_1, x_0 \oplus x_1
\oplus x_2, \ldots, x_0 \oplus x_1 \oplus \ldots \oplus x_{n-2} ]$.

:::::

::::: {#ex-forward-inclusive-scan .example}

**Example:** Forward-inclusive scan

The inclusive form maps a given sequence $[ x_0, x_1, x_2, \ldots,
x_{n-1} ]$ to $[ x_0 \oplus x_1 \oplus \ldots \oplus x_{n-1}, x_{n-2}
\oplus x_{n-1}, x_{n-1} ]$.

:::::

::::: {#ex-forward-exclusive-scan .example}

**Example:** Forward-exclusive scan

The exclusive form maps a given sequence $[ x_0, x_1, x_2, \ldots,
x_{n-1} ]$ to $[ x_0 \oplus x_1 \oplus \ldots \oplus x_{n-2}, x_{n-2}
\oplus x_{n-1}, x_{n-1}, \mathbf{I} ]$.

:::::

The interfact to scan resembles that of `reduce`, except that `scan`
takes one additional argument, `st`, and that the return result is an
array of partial sums rather than just one sum.

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ {.cpp}
template <
  class Iter,
  class Item,
  class Combine
>
parray<Item> scan(Iter lo,
                  Iter hi,
                  Item id,
                  Combine combine,
                  scan_type st);
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Scan has applications in many parallel algorithms. To name just a few,
scan has been used to implement radix sort, search for regular
expressions, dynamically allocate processors, evaluate polynomials,
etc. Suffice to say, scan is important and worth knowing about because
scan is a key component of so many efficient parallel algorithms. In
this course, we are going to study a few more applications not in this
list.

The specification shown above suggests the following sequential
algorithm.

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ {.cpp}
template <class Iter, class Item, class Combine>
void scan_exclusive_seq(Iter lo, Iter hi, Iter dst_lo, Item id, Combine combine) {
  Item acc = id;
  for (Iter it = lo; it != hi; it++, dst_lo++) {
    *dst_lo = acc;
    acc = combine(acc, *it);
  }
}
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

This algorithm is clearly sequential because the result of each
iteration after the first one depends on the result of the previous
iteration. Thus, this exact approach is not likely to lead to a good
parallel solution.

One might then consider a solution such as the following, where we
break the chain of dependencies by doing extra work. The extra work is
represented by the fact that each iteration in the parallel-for loop
performs some additions that are redundant with those of other
iterations.

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ {.cpp}
template <class Iter, class Item, class Combine>
void scan_exclusive(Iter lo, Iter hi, Iter dst_lo, Item id, Combine combine) {
  size_type n = hi - lo;
  if (n == 0) {
    return;
  }
  dst_lo[0] = id;
  parallel_for((size_type)1, n, [&] (size_type i) {
    dst[i] = reduce(lo, lo + i, id, combine);
  });
}
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

::::: {#exercise-scan-blowup .exercise}

**Exercise:** Although the solution above now exposes comparatively
abundant parallelism, it has a serious problem. Think about what it
may be.

:::::

::::: {#exercise-scan-complexity .exercise}

**Exercise:** What is the work complexity of the scan solution above?

:::::

::::: {#exercise-scan-blowup .exercise}

**Exercise:** Do you see the problem with the scan solution now? In
two words, what is that solution *not* that we want the solution to
be?

:::::

Can we do better? Yes, in fact, there exist solutions that take, in
the size of the input, both linear time and logarithmic span, assuming
that the given associative operator takes constant time. It might be
worth pausing for a moment to consider this fact, because the
specification of scan may at first look like it would resist a
solution that is both highly parallel and work efficient.

::::: {#todo-scan-algorithm .todo}

**TODO:** Present a work-efficient scan algorithm and do a more
thorough job with its cost analysis.

:::::

::::: {#todo-scan-dps .todo}

**TODO:** Show destination-passing style version of scan, show how
  it's useful to get the sum of the whole interval in addition to
  getting the exclusive scan in an destination array.

Also, maybe need to introduce the concept of *destination passing
style* in the preliminaries section...

:::::


Derived operations
------------------

Summary of operations
---------------------