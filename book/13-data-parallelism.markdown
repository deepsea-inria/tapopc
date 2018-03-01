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
constant work (and hence, constant span), and then handle the general
case.  In the case where the generator function has constant work
complexity, it should be clear that the tabulation should take work
linear in the size of the array. The reason is that the only work
performed by the body of the loop is performed by the constant-time
generator function. Since the loop itself performs as many iterations
as positions in the array, the work cost is indeed linear in the size
of the array. The span cost of the tabulation is the sum of two
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

Scan
----

Derived operations
------------------

Summary of operations
---------------------