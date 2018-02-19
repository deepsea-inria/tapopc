Preliminaries
=============

Processors, Processes, and Threads
----------------------------------

We assume a machine model that consists of a shared memory by a number
of processors, usually written as $P$. The processors have access to a
shared memory, which is readable and writable by all processors.

We assume that an operating system or a that allows us to create
***processes***. The kernel schedules processes on the available
processors in a way that is mostly out of our control with one
exception: the kernel allows us to create any number of processes and
***pin*** them on the available processors as long as no more than one
process is pinned on a processor.

We define a ***thread*** to be a piece of sequential computation whose
boundaries, i.e., its start and end points, are defined on a case by
case basis, usually based on the programming model. In reality, there
different notions of threads. For example, a system-level thread is
created by a call to the kernel and scheduled by the kernel much like
a process. A user-level thread is created by the application program
and is scheduled by the application—user level threads are invisible
to the kernel. Common property of all threads is that they perform a
sequential computation. In this class, we will usually talk about
user-level threads. In the literature, you will encounter many
different terms for a user-level thread, such as "fiber", "sparc",
"strand", etc.

For our purposes in this book an ***application***, a piece of
runnable software, can only create threads but no processes. We will
assume that we can assign to an application any number of processes to
be used for execution. If an application is run all by itself (without
any other application running at the same time) and if all of its
processes are pinned, then we refer to such an execution as occurring
in the ***dedicated mode***.

::::: {#note001 .note}

*Note:* For now, we leave the details of the memory-consistency model
 unspecified.

:::::

C++ Background
--------------

The material is entirely based on C++ and a library for writing
parallel programs in C++. We use recent features of C++ such as
closures or lambda expressions and templates. A deep understanding of
these topics is not necessary to follow the course notes, because we
explain them at a high level as we go, but such prior knowledge might
be helpful; some pointers are provided below.

### Template metaprogramming

Templates are C++'s way of providing for parametric polymorphism,
which allows using the same code at multiple types. For example, in
modern functional languages such as the ML family or Haskell, you can
write a function $\lambda~x.x$ as an identity function that returns
its argument for any type of $x$. You don't have to write the
function at every type that you plan to apply. Since functional
languages such as ML and Haskell rely on type inference and have
powerful type systems, they can infer from your code the most general
type (within the constraints of the type system). For example, the
function $\lambda~x.x$ can be given the type $\forall \alpha. \alpha
\rightarrow \alpha$. This type says that the function works for any
type $\alpha$ and given an argument of type $\alpha$, it returns a
value of type $\alpha$.

C++ provides for polymorphism with *templates*. In its most basic
form, a template is a class declaration or a function declaration,
which is explicitly stated to be polymorphic, by making explicit the
type variable. Since C++ does not in general perform type inference
(in a rigorous sense of the word), it requires some help from the
programmer.

For example, the following code below defines an array class that is
parametric in the type of its elements. The declaration `template
<class T>` says that the declaration of `class array`, which follows
is parameterized by the identifier `T`. The definition of `class
array` then uses `T` as a type variable. For example, the array
defines a pointer to element sequences of type `T`, and the `sub`
function returns an element of type `T` etc.

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ {.cpp}
template <class T>
class array {
public:
 array (int size) { a = new T[size]; }
 T sub (int i) { a[i]; }

private:
  *T a;
};
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Note that the only part of the syntax `template <class T>` that is
changeable is the identifier `T`. In other words, you should think of
the syntax `template <class T>` as a binding form that allows you to
pick an identifier (in this case `T`). You might ask why the type
identifier/variable `T` is a `class`. This is a good question. The
authors find it most helpful to not think much about such questions,
especially in the context of the C++ language.

Once defined a template class can be initialized with different type
variables by using the `< >` syntax. For examples, we can define
different arrays such as the following.

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ {.cpp}
array<int> myFavoriteNumbers(7); 
array<char*> myFavoriteNames(7); 
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Again, since C++ does not perform type inference for class instances,
the C++ compiler expects the programmer to eliminate explicitly
parametricity by specifying the argument type.

It is also possible to define polymorphic or generic functions. For
example, the following declarations defines a generic identity
function.

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ {.cpp}
template <class T>
T identity(T x) { return x;} 
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Once defined, this function can be used without explicitly
specializing it at various types. In contrast to templated classes,
C++ does provide some type inference for calls to templated functions.
So generic functions can be specialized implicitly, as shown in the
examples below.

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ {.cpp}
i = identity (3) 
s = identity ("template programming can be ugly") 
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

This brief summary of templates should suffice for the purposes of the
material covered in this book. Templates are covered in significant
detail by many books, blogs, and discussions boards. We refer the
interested reader to those sources for further information.

### Lambda expressions

The C++11 reference provides good documentation on
[lambda expressions](http://en.cppreference.com/w/cpp/language/lambda).
