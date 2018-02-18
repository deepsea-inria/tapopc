Preface
=======

The goal of these notes is to introduce the reader to the following.

1. Parallel computing in imperative programming languages and C++ in
   particular, and
2. Real-world performance and efficiency concerns in writing parallel
   software and techniques for dealing with them.

For parallel programming in C++, we use a library, called the Series
Parallel Template Library (SPTL, from hereon), that we have been
developing over the past three years. The implementation of the
library uses advanced scheduling techniques to run parallel programs
efficiently on modern multicores and provides a range of utilities for
understanding the behavior of parallel programs.

In this context, we use series parallel to refer to fork-join
parallelism, the only form of parallelism addressed in this
course. Fork join makes for a good entry point into parallel
programming because divide-and-conquer algorithms often admit elegant
parallel solutions in fork-join style. Other important, but more
advanced, forms of parallelism, such as parallel pipelining,
SIMD-level parallelism, and speculative parallelism, are not covered
in this book.

We expect that the instructions in this book and SPTL to allow the
reader to write high-performance parallel programs at a relatively
high level (essentially at the same level of C++ code) without having
to worry too much about lower level details such as machine-specific
optimizations, which might otherwise be necessary.

All code that associated with this book can be found at the Github
repository linked by the following URL:

[https://github.com/deepsea-inria/sptl](https://github.com/deepsea-inria/sptl)

This book does not focus on the design and analysis of parallel
algorithms. The interested reader can find more details this topic in
[this book](http://www.parallel-algorithms-book.com/).
