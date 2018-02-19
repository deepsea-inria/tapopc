Introduction
============

This class is motivated by recent advances in architecture that put
sequential hardware on the path to extinction. Due to fundamental
architectural limitations, sequential performance of processors have
not been increasing since 2005. In fact performance has been
decreasing by some measures because hardware manufacturer's have been
simplifying processorns by simplifying the handling of instruction
level parallelism.

Moore's law, however, continues to be holding, at least for the time
being, enabling hardware manufacturers to squeeze an increasing number
of transistors into the same chip area.  The result, not surprisingly,
has been increased paralellelism, more processors that is. In
particular, manufacturers have been producing multicore chips where
each chip consists of a number of processors that fit snugly into a
small area, making communication between them fast and efficient.  You
can read more about the [history of modern multicore
computers](https://www.google.com/search?q=history+of+multicore+processors&gws_rd=ssl).

This simple change in hardware has led to dramatic changes in
computing.  Parallel computing, once a niche domain for computational
scientists, is now an everyday reality.  Essentially all computing
media ranging from mobile phones to laptops and computers operate on
parallel computers.

This change was anticipated by computers scientists.  There was in
fact much work on [parallel
algorithms](https://www.google.com/search?q=parallel+algorithms&gws_rd=ssl)
in 80's and 90's.  The models of computation assumed then turned out
to be unrealistic.  This makes it somewhat of a challenge to use the
algorithms from that era. Some of the ideas, however, transcends those
earlier models can still be used today to design and implement
parallel algorithms on modern architectures.

The goal of this class is to give an introduction to the theory and
the practice of parallol computing.  Specifically, we will cover the
following topics.

1. Multithreaded computation
2. Work and span
3. Offline scheduling
4. Structured or implicit parallel computation
   - Fork-join, async-finish, nested parallelism.
   - Futures.
   - Parallelism versus concurrency
5. Parallel algorithms for sequences
6. Online scheduling: work-stealing algorithms and its analysis
7. Parallel algorithms for trees
8. Parallel algorithms for graphs
9. Concurrency
