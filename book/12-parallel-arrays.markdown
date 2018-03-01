Parallel arrays
===============

Arrays are a fundamental data structure in sequential and parallel
computing.  When computing sequentially, arrays can sometimes be
replaced by linked lists, especially because linked lists are more
flexible.  Despite their flexibility in the sequential setting, linked
lists are deadly for parallelism. The reason is that linked lists
impose serial traversals to find elements. Because it does not apply
to arrays, this limitation makes arrays all the more important in
parallel computing.

Unfortunately, however, it is difficult to find a good treatment of
parallel arrays in C++, because the various array implementations
provided by C++ have been designed primarily for sequential
computing. Each one has various pitfalls for parallel use. Because
these pitfalls are instructive, we examine them first and then
consider the parrallel array interface we are going to use.

::::: {#ex-cpp-arrays .example}

**Example:** native C++ arrays

By default, C++ arrays that are created by the `new[]` operator are
initialized sequentially. Therefore, the work and span cost of the
call `new[n]` is `n`, i.e., fully serial. Yet, we could easily have a
parallel solution that can initialize an array in logarithmic span in
the number of items.

:::::

::::: {#ex-cpp-vector .example}

**Example:** the STL vector

The vector data structure that is provided by the Standard Template
Library (STL) has similar issues.  The STL vector implements a
dynamically resizable array that provides push, pop, and indexing
operations. The push and pop operations take amortized constant time
and the indexing operation constant time.  As with C++ arrays,
initialization of vectors can require linear work and span.  The STL
vector also provides the method `resize(n)` which changes the size of
the array to be `n`. The resize operation takes, in the worst case,
linear work and span in proportion to the new size, `n`. In other
words, the resize function uses a sequential algorithm to fill the
cells in the vector. The `resize` operation is therefore not parallel
for the same reason as for the default C++ arrays.

:::::

Such sequential computations that exist behind the wall of abstraction
of a language or library can harm parallelism by introducing implicit
sequential dependencies. Finding the source of such sequential
bottlenecks can be time consuming, because they are hidden behind the
abstraction boundary of the native array abstraction that is provided
by the programming language.

We can avoid such pitfalls by carefully designing our own array data
structure. Because array implementations are quite subtle, we consider
the implementation of parallel arrays in our SPTL. The SPTL array
implementation makes explicit the cost of array operations, thereby
allowing us to control them quite carefully.  Specifically, we design
the array operations to carefully control initialization and to
disallow implicit copy operations on arrays, because copy operations
can harm observable work efficiency (their asymptotic work cost is
linear).

Interface and cost model
------------------------

The key components of our array data structure, namely `parray`, are
shown by the code snippet below. A `parray` can store items of type
`Item`.

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ {.cpp}
template <class Item>
class parray {
public:

  // n: size to give to the array; by default 0
  parray(size_type n = 0);

  // constructor from list
  parray(std::initializer_list<Item> xs);

  // indexing operator
  Item& operator[](size_type i);

  // size of the array
  size_type size() const;

};
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

The class `parray` provides two constructors.  The first one takes in
the size of the array (set to 0 by default) and allocates an
unitialized array of the specified size (`nullptr` if size is 0).  The
second constructor takes in a list specified by curly braces and
allocates an array with the same size.  Since the argument to this
constructor must be specified explicitly in the program, its size is
constant by definition.

The cost model guaranteed by our implementation of parallel array is
as follows:

- *Constructors/Allocation:* The work and span of simply allocating an
  array on the heap, without initialization, is constant.  The second
  constructor performs initialization, based on constant-size lists,
  and thus also has constant work and span.

- *Array indexing:* Each array-indexing operation, that is the
  operation which accesses an individual cell, requires constant work
  and constant span.

- *Size operation:* The work and the span of accessing the size of the
  array is constant.

- *Destructors/Deallocation:* Not shown, the class includes a
   destructor that frees the array.  Combined with the "move
   assignment operator" that C++ allows us to define, destructors can
   be used to deallocate arrays when they are out of scope. The
   destructor takes constant time if the items stored in the array
   have trivial deconstructors (i.e., are primitive types, such as
   `int`).  Otherwise, for items that have non-trivial, but
   constant-time destructors, the work is linear in the number of
   items being destroyed and the span logarithmic. The work and span
   costs generalize in a straightforward fashion when the destructor
   takes non-constant time.

- *Move assignment operator:* Not shown, the class includes a
   move-assignment operator that gets fired when an array is assigned
   to a variable.  This operator moves the contents of the right-hand
   side of the assigned array into that of the left-hand side. This
   operation takes constant time.

- *Copy constructor:* The work and span costs of the copy constructor
  are typically linear and logarithmic, respectively, in proportion to
  the total amount of data being copied.

::::: {#ex-parray-simple-use .example}

**Example:** Simple use of `parray`s

The first line allocates and initializes the contents of the array to
be three numbers. The second uses the familiar indexing operator to
access the item at the second position in the array. The third line
extracts the size of the array. The fourth line assigns to the second
cell the value 5. The fifth prints the contents of the cell.

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ {.cpp}
parray<int> xs = { 1, 2, 3 };
std::cout << "xs[1] = " << xs[1] << std::endl;
std::cout << "xs.size() = " << xs.size() << std::endl;
xs[2] = 5;
std::cout << "xs[2] = " << xs[2] << std::endl;
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Output:

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
xs[1] = 2
xs.size() = 3
xs[2] = 5
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

:::::

Allocation and deallocation
---------------------------

Arrays can be allocated by specifying the size of the array.

::::: {#ex-parray-allocation .example}

**Example:** Allocation

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ {.cpp}
parray<int> zero_length = parray<int>();
parray<int> another_zero_length = parray<int>(0);
parray<int> yet_another_zero_length;
parray<int> length_five(5, [&] (size_type i) { return i; });
std::cout << "|zero_length| = " << zero_length.size() << std::endl;
std::cout << "|another_zero_length| = " << another_zero_length.size() << std::endl;
std::cout << "|yet_another_zero_length| = " << yet_another_zero_length.size() << std::endl;
std::cout << "|length_five| = " << length_five.size() << std::endl;
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Output:

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
|zero_length| = 0
|another_zero_length| = 0
|yet_another_zero_length| = 0
|length_five| = 5
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

:::::

Internally, the `parray` class consists of a size field and a pointer
to the first item in the array. The contents of the array are heap
allocated (automatically) by constructor of the `parray` class.
Deallocation occurs when the array's destructor is called.  The
destructor can be called by the programmer or by run-time system (of
C++) if an object storing the array is destructed.  Since C++
destructs (stack allocated) variables that go out of scope when a
function returns, we can combine the stack discipline with
heap-allocated arrays to manage the deallocation of arrays mostly
automatically.  We give several examples of this automatic
deallocation scheme below.

::::: {#ex-parray-deallocation .example}

**Example:** Automatic deallocation of arrays upon return

In the function below, the `parray` object that is allocated on the
frame of `foo` is deallocated just before `foo` returns, because the
variable `xs` containing it goes out of scope.

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ {.cpp}
void foo() {
  parray<int> xs = parray<int>(10);
  // array deallocated just before foo() returns
}
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

:::::

::::: {#ex-dangling-pointers .example}

**Example:** Dangling pointers in arrays

Care must be taken when managing arrays, because nothing prevents the
programmer from returning a dangling pointer.

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ {.cpp}
int* foo() {
  parray<int> xs(10, [&] (size_type i) { return i; });
  ...
  // array deallocated just before foo() returns
  return &xs[0]
}

std::cout << "contents of deallocated memory: " << *foo() << std::endl;
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Output:

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
contents of deallocated memory: .... (undefined)
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

It is safe to take a pointer to a cell in the array, when the array
itself is still in scope.  For example, in the code below, the
contents of the array are used strictly when the array is in scope.

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ {.cpp}
void foo() {
  parray<int> xs(10, [&] (size_type i) { return i; });
  xs[0] = 34;
  bar(&xs[0]);
  ...
  // array deallocated just before foo() returns
}

void bar(int* p) {
  std::cout << "xs[0] = " << *p << std::endl;
}
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Output:

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
xs[0] = 34
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

:::::

Next, we are going to see that we can rely on cleaner conventions for
passing to functions references on arrays.

Passing to and returning from functions
---------------------------------------

C++ uses the call-by-value method for passing arguments to a
function. This method is simple and works well for small
values. However, call by value is rarely used when passing arrays
because to pass an array by value requires the caller to pay the
linear cost of making a copy of the array to be passed. Nevertheless,
there are some circumstances where using call by value is useful.

::::: {#ex-pass-by-copy .example}

In the code below, the call to `foo` passes the array argument by
value.

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ {.cpp}
int foo(parray<int> xs) {
  return xs[0];
}

void bar() {
  parray<int> xs = { 1, 2 };
  foo(xs);
}
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

What do you think should be the work and span cost of the call by
value of an array of $n$ 32-bit ints?

:::::

Fortunately, in C++, one can instead *pass by reference*. To pass by
reference means that, instead of passing a copy, the caller passes to
the callee a pointer to the value being passed. The advantage of
passing by reference is, as such, that the operation takes constant
time as opposed to linear for the copy.

::::: {#ex-pass-by-reference .example}

This example uses call by reference because of the `&` annotation
appearing in the parameter of `foo`.

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ {.cpp}
int foo(parray<int>& xs) {
  return xs[0];
}

void bar() {
  parray<int> xs = { 1, 2 };
  foo(xs);
}
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

:::::

Returning an array is straightforward: we take advantage of a feature
of modern C++11 which automatically detects when it is safe to move a
structure by a constant-time pointer swap. Code of the following form
is perfectly legal, even though we disabled the copy constructor of
`parray`, because the compiler is able to transfer ownership of the
array to the caller of the function. Moreover, the transfer is
guaranteed to be constant work -- not linear like a copy would
take. The return is fast, because internally all that happens is that
a couple words are being exchanged.  Such "move on return" is achieved
by the "move-assignment operator" of `parray` class.

::::: {#ex-parray-create-init .example}

**Example:** Create and initialize an array (sequentially)

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ {.cpp}
parray<int> fill_seq(size_type n, int x) {
  parray<int> tmp(n);
  for (size_type i = 0; i < n; i++) {
    tmp[i] = x;
  }
  return tmp;
}

void bar() {
  parray<int> xs = fill_seq(4, 1234);
  std::cout << "xs = " << xs << std::endl;
}
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Output after calling `bar()`:

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
xs = { 1234, 1234, 1234, 1234 }
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

:::::

Although it is perfectly fine to assign to an array variable the
contents of a given array, what happens may be surprising to those who
know the usual conventions of C++11 container libraries.  Consider the
following program.

::::: {#ex-parray-move .example}

**Example:** Move semantics

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ {.cpp}
parray<int> xs = fill_seq(4, 1234);
parray<int> ys = fill_seq(3, 333);
ys = std::move(xs);
std::cout << "xs = " << xs << std::endl;
std::cout << "ys = " << ys << std::endl;
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

The assignment from `xs` to `ys` simultaneously destroys the contents
of `ys` (by calling its destructor, which nulls it out), namely the
array `{ 333, 333, 333 }`, moves the contents of `xs` to `ys`, and
empties out the contents of `xs`.  This behavior is defined as part of
the move operator of `parray`.  The result is the following.

Output:

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
xs = { }
ys = { 1234, 1234, 1234, 1234 }
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

:::::

The reason we use this semantics for assignment is that the assignment
operation is trivial and fast: a move takes constant time. Later, we
are going to see that we can efficiently copy items out of an
array. But for reasons we already discussed, the copy operation is
going to be explicit.

::::: {#exercise-duplicating .exercise}

**Exercise:** duplicating items in parallel

The aim of this exercise is to combine our knowledge of parallelism
and arrays. To this end, the exercise is to implement two
functions. The first, namely `duplicate`, is to return a new array in
which each item appearing in the given array `xs` appears twice.

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ {.cpp}
parray<int> duplicate(const parray<int>& xs) {
  // fill in
}
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ {.cpp}
parray<int> xs = { 1, 2, 3 };
std::cout << "xs = " << duplicate(xs) << std::endl;
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Expected output:

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
xs = { 1, 1, 2, 2, 3, 3 }
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

The second function is a generalization of the first: the value
returned by `ktimes` should be an array in which each item `x` that is
in the given array `xs` is replaced by `k` duplicate items.

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ {.cpp}
parray<int> ktimes(const parray<int>& xs, size_type k) {
  // fill in
}
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

For example:

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ {.cpp}
sparray xs = { 5, 7 };
std::cout << "xs = " << ktimes(xs, 3) << std::endl;
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Expected output:

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
xs = { 5, 5, 5, 7, 7, 7 }
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Notice that the `k` parameter of `ktimes` is not bounded. Your
solution to this problem should be highly parallel not only in the
number of items in the input array, `xs`, but also in the
duplication-degree parameter, `k`.

- What is the work and span complexity of your solution?

- Does your solution expose ample parallelism? How much, precisely?

- What is the speedup do you observe in practice on various input
  sizes?

:::::

Iterators
---------

