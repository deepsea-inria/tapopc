Critical Sections and Mutual Exclusion
======================================

In a multithreaded program, a ***critical section*** is a part of the
program that may not be executed by more than one thread at the same
time. Critical sections typically contain code that alters shared
objects, such as shared (e.g., global) variables.This means that the
a critical section requires ***mutual exclusion***: only one thread can
be inside the critical section at any time. 

Since only one thread can be inside a critical section at a time,
threads must coordinate to make sure that they don't enter the
critical section at the same time. If threads do not coordinate and
multiple threads enter the critical section at the same time, we say
that a ***race condition*** occurs, because the outcome of the program
depends on the relative timing of the threads, and thus can vary from
one execution to another. Race conditions are sometimes benign but
usually not so, because they can lead to incorrect behavior.
Spectacular examples of race conditions' effects include the
"Northeast Blackout" of 2003, which affected 45 million people in the
US and 10 million people in Canada. 

It can be extremely difficult to find a race condition, because of the
non-determinacy of execution. A race condition may lead to an
incorrect behavior only a tiny fraction of the time, making it
extremely difficult to observe and reproduce it. For example, the
software fault that lead to the Northeast blackout took software
engineers "weeks of poring through millions of lines of code and data
to find it" according to one of the companies involved.

The problem of designing algorithms or protocols for ensuring mutual
exclusion is called the ***mutual exclusion problem*** or the ***critical
section*** problem. There are many ways of solving instances of the
mutual exclusion problem. But broadly, we can distinguish two
categories: spin-locks and blocking-locks. The idea in ***spin locks***
is to busy wait until the critical section is clear of other threads.
Solutions based on ***blocking locks*** is similar except that instead
of waiting, threads simply block. When the critical section is clear,
a blocked thread receives a signal that allows it to proceed. The
term ***mutex***, short for "mutual exclusion" is sometimes used to
refer to a lock.

Mutual exclusions problems have been studied extensively in the
context of several areas of computer science. For example, in
operating systems research, processes, which like threads are
independent threads of control, belonging usually but not always to
different programs, can share certain systems' resources. To enable
such sharing safely and efficiently, researchers have proposed various
forms of locks such as ***semaphores***, which accepts both a
busy-waiting and blocking semantics. Another class of locks, called
***condition variables*** enable blocking synchronization by
conditioning an the value of a variable.


Parallelism and Mutual Exclusion
--------------------------------

In parallel programming, mutual exclusion problems do not have to
arise. For example, if we program in a purely functional language
extended with structured multithreading primitives such as fork-join
and futures, programs remain purely functional and mutual-exclusion
problems, and hence race conditions, do not arise. If we program in an
imperative language, however, where memory is always a shared
resource, even when it is not intended to be so, threads can easily
share memory objects, even unintentionally, leading to race
conditions.

::::: {#ex-writing-same-loc .example}

**Example:** Writing to the same location in parallel

In the code below, both branches of `fork2` are writing into `b`.
What should then the output of this program be?

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ {.cpp}
long b = 0;

fork2([&] {
  b = 1;
}, [&] {
  b = 2;
});

std::cout << "b = " << std::endl;
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

At the time of the print, the contents of `b` is determined by the
last write. Thus depending on which of the two branches perform the
write, we can see both possibilities:

Output:

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
b = 1
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Output:

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
b = 2
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

:::::

::::: {#ex-fib2 .example}

**Example:** Fibonacci

Consider the following alternative implementation of the Fibonacci
function. By "inlining" the plus operation in both branches, the
programmer got rid of the addition operation after the `fork2`.

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ {.cpp}
long fib_par_racy(long n) {
  long result = 0;
  if (n < 2) {
    result = n;
  } else {
    fork2([&] {
      result += fib_par_racy(n-1);
    },[&] {
      result += fib_par_racy(n-2);
    });
  }
  return result;
}
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

This code is not correct because it has a race condition.

:::::

As the example shows, separate threads are updating the value result
but it might look like this is not a race condition because the update
consists of an addition operation, which reads the value and then
writes to `result`. The race condition might be easier to see if we
expand out the applications of the `+=` operator.

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ {.cpp}
long fib_par_racy(long n) {
  long result = 0;
  if (n < 2) {
    result = n;
  } else {
    fork2([&] {
      long a1 = fib_par_racy(n-1);
      long a2 = result;
      result = a1 + a2;
    },[&] {
      long b1 = fib_par_racy(n-2);
      long b2 = result;      
      result = b1 + b2;
    });
  }
  return result;
}
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

When written in this way, it is clear that these two parallel threads
are not independent: they both read `result` and write to
`result`. Thus the outcome depends on the order in which these reads
and writes are performed, as shown in the next example.

::::: {#ex-trace-race .example}

**Example:** Execution trace of a race condition

The following table takes us through one possible execution trace of
the call `fib_par_racy(2)`. The number to the left of each instruction
describes the time at which the instruction is executed. Note that
since this is a parallel program, multiple instructions can be
executed at the same time. The particular execution that we have in
this example gives us a bogus result: the result is 0, not 1 as it
should be.


Time step   Thread 1                Thread 2
---------   --------                ---------
1           a1 = fib_par_racy(1)    b2 = fib_par_racy(0)
2           a2 = result             b3 = result
3           result = a1 + a2        _
4           _                       result = b1 + b2

The reason we get a bogus result is that both threads read the initial
value of result at the same time and thus do not see each others
write. In this example, the second thread "wins the race" and writes
into `result`. The value 1 written by the first thread is effectively
lost by being overwritten by the second thread.

:::::

Synchronization Hardware
------------------------

Since mutual exclusion is a common problem in computer science, many
hardware systems provide specific synchronization operations that can
help solve instances of the problem. These operations may allow, for
example, testing the contents of a (machine) word then modifying it,
perhaps by swapping it with another word. Such operations are
sometimes called atomic ***read-modify-write*** or ***RMW***, for short,
operations.

A handful of different RMW operations have been proposed. They include
operations such as ***load-link/store-conditional***, ***fetch-and-add***,
and ***compare-and-swap***. They typically take the memory location `x`,
and a value `v` and replace the value of stored at `x` with
`f(x,v)`. For example, the fetch-and-add operation takes the location
`x` and the increment-amount, and atomically increments the value at
that location by the specified amount, i.e., `f(x,v) = *x + v`.

The compare-and-swap operation takes the location `x` and takes a pair
of values `(a,b)` as the second argument, and stores `b` into `x` if
the value in `x` is `a`, i.e., `f(x,(a,b)) = if *x = a then b else a`;
the operation returns a Boolean indicating whether the operation
successfully stored a new value in `x`. The operation
"compare-and-swap" is a reasonably powerful synchronization operation:
it can be used by arbitrarily many threads to agree (reach consensus)
on a value. This instruction therefore is frequently provided by
modern parallel architectures such as Intel's X86. 

In C++, the `atomic` class can be used to perform synchronization.
Objects of this type are guarantee to be free of race conditions; and
in fact, in C++, they are the only objects that are guaranteed to be
free from race conditions. The contents of an `atomic` object can be
accessed by `load` operations, updated by `store` operation, and also
updated by `compare_exchange_weak` and `compare_exchange_strong`
operations, the latter of which implement the compare-and-swap
operation.

::::: {#ex-accessing-cells .example}

**Example:** Accessing the contents of atomic memory cells

Access to the contents of any given cell is achieved by the `load()`
and `store()` methods.

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ {.cpp}
std::atomic<bool> flag;

flag.store(false);
std::cout << flag.load() << std::endl;
flag.store(true);
std::cout << flag.load() << std::endl;
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Output:

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
0
1
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

:::::

The key operation that help with race conditions is the atomic
compare-and-exchange operation, aka, atomic compare and swap.

::::: {#definition-compare-swap .definition}

**Definition:** compare and swap

When executed with a `target` atomic object and an `expected` cell and
a new value `new' this operation performs the following steps,
atomically:

- Read the contents of `target`.
- If the contents equals the contents of `expected`, then writes `new`
  into the `target` and returns `true`.
- Otherwise, returns `false`.

:::::

::::: {#ex-reading-writing-atomics .example}

**Example:** Reading and writing atomic objects

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ {.cpp}
std::atomic<bool> flag;

flag.store(false);
bool expected = false;
bool was_successful = flag.compare_exchange_strong(expected, true);
std::cout << "was_successful = " << was_successful << "; flag = " << flag.load() << std::endl;
bool expected2 = false;
bool was_successful2 = flag.compare_exchange_strong(expected2, true);
std::cout << "was_successful2 = " << was_successful2 << "; flag = " <<
flag.load() << std::endl;
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Output:

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
was_successful = 1; flag = 1
was_successful2 = 0; flag = 1
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

:::::

As another example use of the `atomic` class, recall our Fibonacci
example with the race condition. In that example, race condition
arises because of concurrent writes to the `result` variable. We can
eliminate this kind of race condition by using different memory
locations, or by using an atomic class and using a
`compare_exchange_strong` operation.

::::: {#ex-fib3 .example}

**Example:** Fibonacci

The following implementation of Fibonacci is not safe because the
variable `result` is shared and updated by multiple threads.

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ {.cpp}
long fib_par_racy(long n) {
  long result = 0;
  if (n < 2) {
    result = n;
  } else {
    fork2([&] {
      result += fib_par_racy(n-1);
    },[&] {
      result += fib_par_racy(n-2);
    });
  }
  return result;
}
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

We can solve this problem by declaring `result` to be an atomic type
and using a standard busy-waiting protocol based on compare-and-swap.

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ {.cpp}
long fib_par_atomic(long n) {
  atomic<long> result = 0;
  if (n < 2) {
    result.store(n);
  } else {
    fork2([&] {
      long r = fib_par_racy(n-1);
      // Atomically update result.
      while (true) {
        long exp = result.load();
        bool flag = result.compare_exchange_strong(exp,exp+r)
        if (flag) {break;}
      }
    },[&] {
      long r = fib_par_racy(n-2);
      // Atomically update result.
      while (true) {
        long exp = result.load();
        bool flag = result.compare_exchange_strong(exp,exp+r)
        if (flag) {break;}
      }
    });
  }
  return result;
}
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

The idea behind the solution is to load the current value of `result`
and atomically update `result` only if it has not been modified (by
another thread) since it was loaded. This guarantees that the `result`
is always updated (read and modified) correctly without missing an
update from another thread.

:::::

The example above illustrates a typical use of the compare-and-swap
operation. In this particular example, we can probably prove our code
is correct. But this is not always as easy due to a problem called the
"ABA problem."

ABA problem 
-----------

While reasonably powerful, compare-and-swap suffers from the so-called
***ABA problem***. To see this consider the following scenario where a
shared variable `result` is update by multiple threads in parallel: a
thread, say $T$, reads the `result` and stores its current value, say
`2`, in `current`. In the mean time some other thread also reads
`result` and performs some operations on it, setting it back to `2`
after it is done. Now, thread $T$ takes its turn again and attempts to
store a new value into `result` by using `2` as the old value and
being successful in doing so, because the value stored in `result`
appears to have not changed. The trouble is that the value has
actually changed and has been changed back to the value that it used
to be. Thus, compare-and-swap was not able to detect this change
because it only relies on a simple shallow notion of equality. If for
example, the value stored in `result` was a pointer, the fact that the
pointer remains the same does not mean that values accessible from the
pointer has not been modified; if for example, the pointer led to a
tree structure, an update deep in the tree could leave the pointer
unchanged, even though the tree has changed.

This problem is named as such, because the ABA problem involves
cycling the atomic memory between the three values $A$, $B$, and again
$A$). The ABA problem is an important limitation of compare-and-swap:
the operation itself is not atomic but is able to behave as if it is
atomic if it can be ensured that the equality test of the subject
memory cell suffices for correctness.

In the example below, ABA problem may happen (if the counter is
incremented and decremented again in between a load and a store) but
it is impossible to observe because it is harmless. If however, the
compare-and-swap was on a memory object with references, the ABA
problem could have had observable effects.

The ABA problem can be exploited to give seemingly correct
implementations that are in fact incorrect. To reduce the changes of
bugs due to the ABA problem, memory objects subject to
compare-and-swap are usually tagged with an additional field that
counts the number of updates. This solves the basic problem but only
up to a point because the counter itself can also wrap around. The
load-link/store-conditional operation solves this problem by
performing the write only if the memory location has not been updated
since the last read (load) but its practical implementations are hard
to come by.
