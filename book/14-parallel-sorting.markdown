Parallel sorting
================

In this chapter, we are going to study parallel implementations of
quicksort and mergesort.

Quicksort
---------

The quicksort algorithm for sorting an array (sequence) of elements is
known to be a very efficient sequential sorting algorithm.  A natural
question thus is whether quicksort is similarly effective as a
parallel algorithm?

Let us first convince ourselves, at least informally, that quicksort is
actually a good parallel algorithm.  But first, what do we mean by
"parallel quicksort."  Chances are that you think of quicksort as an
algorithm that, given an array, starts by reorganizing the elements in
the array around a randomly selected pivot by using an in-place
*partitioning* algorithm, and then sorts the two parts of the array to
the left and the right of the array recursively.

While this implementation of the quicksort algorithm is not
immediately parallel, it can be parallelized.  Note that the recursive
calls are naturally independent.  So we really ought to focus on the
partitioning algorithm.  There is a rather simple way to do such a
partition in parallel by performing three `filter` calls on the input
array, one for picking the elements less that the pivot, one for
picking the elements equal to the pivot, and another one for picking
the elements greater than the pivot.  This algorithm can be described
as follows.

::::: {#algorithm-quicksort .algorithm}

**Algorithm:** parallel quicksort

- Pick from the input sequence a pivot item.

- Based on the pivot item, create a three-way partition of the input
     sequence:
  - the sequence of items that are less than the pivot item,
  - those that are equal to the pivot item, and
  - those that are greater than the pivot item.

- Recursively sort the "less-than" and "greater-than" parts

- Concatenate the sorted arrays. 

:::::
 
Now that we have a parallel algorithm, we can check whether it is a
good algorithm or not.  Recall that a good parallel algorithm is one
that has the following three characteristics

- It is asymptotically work efficient

- It is observably work efficient

- It is highly parallel, i.e., has low span.

Let us first convince ourselves that Quicksort is a highly parallel
algorithm.  Observe that

- the dividing process is highly parallel because no dependencies
  exist among the intermediate steps involved in creating the
  three-way partition,

- two recursive calls are parallel, and

- concatenations are themselves highly parallel.

### Asymptotic Work Efficiency and Parallelism

Let us now turn our attention to asymptotic and observed work
efficiency.  Recall first that quicksort can exhibit a quadratic-work
worst-case behavior on certain inputs if we select the pivot
deterministically.  To avoid this, we can pick a random element as a
pivot by using a random-number generator, but then we need a parallel
random number generator.  Here, we are going to side-step this issue
by assuming that the input is randomly permuted in advance. Under this
assumption, we can simply pick the pivot to be the first item of the
sequence.  With this assumption, our algorithm performs asymptotically
the same work as sequential quicksort implementations that perform
$\Theta(n\log{n})$ in expectation.

For the analysis, let's assume a version of quicksort that compares
the pivot $p$ to each key in the input once (instead of 3 times.  The
figure below illustrates the structure of an execution of quicksort by
using a tree.  Each node corresponds to a call to the quicksort
function and is labeled with the key at that call. Note that the tree
is a binary search tree.

![Quicksort call tree](images/qsort-bst-example.jpg)

Let's observe some properties of quicksort.  

- In quicksort, a comparison always involves a pivot and another
  key. Since, the pivot is never sent to a recursive call, a key is
  selected as a pivot exactly once, and is not involved in further
  comparisons (after is becomes a pivot). Before a key is selected as
  a pivot, it may be compared to other pivots, once per pivot, and
  thus two keys are never compared more than once.

- When the algorithm selects a key $y$ as a pivot and if $y$ is
  between two other keys $x, z$ such that $x < y < z$, it sends the
  two keys $y, z$ to two separate subtrees.  The two keys $x$ and $z$
  separated in this way are never compared again.

- We can sum up the two observations: a key is compared with all its
  ancestors in the call tree and all its descendants in the call tree,
  and with no other keys.

Since a pair of key are never compared more than once, the total
number of comparisons performed by quicksort can be expressed as the
sum over all pairs of keys.

Let $X_n$ be the random variable denoting the total number of
comparisons performed in an execution of quicksort with a randomly
permuted input of size $n$.

We want to bound the expectation of $X_n$, $E \lbrack X_n \rbrack$.

For the analysis, let's consider the final sorted order of the keys
$T$, which corresponds to the output.  Consider two positions $i, j
\in \{1, \dots, n\}$ in the sequence $T$. We define following random
variable:

$$
\begin{array}{lll} 
A_{ij} & = &\left\{
\begin{array}{ll} 
1 & \mbox{if}~T_i~\mbox{and}~T_j \mbox{are compared}\\ 
0  & \mbox{otherwise} 
\end{array}
\right.  
\end{array}
$$

We can write $X_n$ by summing over all $A_{ij}$'s: 

$$
X_n \leq \sum_{i=1}^n \sum_{j=i+1}^n A_{ij} 
$$

By linearity of expectation, we have
$$
E \lbrack X_n \rbrack 
\leq 
\sum_{i=1}^n \sum_{j=i+1}^n E \lbrack A_{ij}  \rbrack
$$

Furthermore, since each $A_{ij}$ is an indicator random variable, $E
\lbrack A_{ij} \rbrack = P(A_{ij}) = 1$.  Our task therefore comes
down to computing the probability that $T_i$ and $T_j$ are compared,
i.e., $P(A_{ij} = 1)$, and working out the sum.

![Relationship between the pivot and other keys.](images/qsort-cases.jpg)

To compute this probability, note that each call takes the pivot $p$
and splits the sequence into two parts, one with keys larger than $p$
and the other with keys smaller than $p$. For any one call to
quicksort there are three possibilities as illustrated in the figure
above.

- The pivot is (equal to) either $T_i$ or $T_j$, in which case $T_i$
  and $T_j$ are compared and $A_{ij} = 1$.  Since there are $j-i+1$
  keys in the interval $T_i \ldots T_j$ and since each one is equally
  likely to be the first in the randomly permuted input, the $P(A_{ij}
  = 1) = \frac{2}{j-i+1}$.

- The pivot is a key between $T_i$ and $T_j$, and $T_i$ and $T_j$ will
  never be compared; thus $A_{ij} = 0$.

- The pivot is less than $T_i$ or greater than $T_j$.  Then $T_i$ and
  $T_j$ are sent to the same recursive call.  Whether $T_i$ and $T_j$
  are compared will be determined in some later call.

Hence, the expected number of comparisons made in randomized quicksort
is

$$
\begin{array}{ll}
E \lbrack   X_n  \rbrack 
& 
\leq \sum_{i=1}^{n-1} \sum_{j=i+1}^n E \lbrack A_{ij}  \rbrack
\\
& 
= \sum_{i=1}^{n-1}  \sum_{j=i+1}^n \frac{2}{j-i+1}
\\
&
= \sum_{i=1}^{n-1} n \sum_{k=2}^{n-i+1} \frac{2}{k} 
\\
&
\leq 2\sum_{i=1}^{n-1}  H_n 
\\
& = 
2nH_n \in O(n\log n)
\end{array}
$$

The last step follows by the fact that $H_n = \ln{n} + O(1)$.

Having completed work, let's analyze the span of quicksort.  Recall
that each call to quicksort partitions the input sequence of length
$n$ into three subsequences $L$, $E$, and $R$, consisting of the
elements less than, equal to, and greater than the pivot.

Let's first bound the size of the left sequence $L$.

$$
\begin{array}{ll}
E \lbrack |L| \rbrack
& 
= \sum_{i=1}^{n-1}{\frac{i-1}{n}}
\\
&
\le \frac{n}{2}.
\end{array}
$$

By symmetry, $E \lbrack |R| \rbrack \le \frac{n}{2}$.  This reasoning
applies at any level of the quicksort call tree.  In other words, the
size of the input decreases by $1/2$ in expectation at each level in
the call tree.

Since pivot choice at each call is independent of the other calls.
The expected size of the input at level $i$ is $E \lbrack Y_i \rbrack
= \frac{n}{2^i}$.

Let's calculate the expected size of the input at depth $i = 5\lg{n}$.
By basic arithmetic, we obtain

$$
E \lbrack Y_{5\lg{n}} \rbrack 
= 
n\, \frac{1}{2^{5\lg{n}}} = n\, n^{-5\lg{2}} = n^{-4}. 
$$

Since $Y_i$'s are always non-negative, we can use [Markov's
inequality](https://en.wikipedia.org/wiki/Markov%27s_inequality), to
turn expectations into probabilities as follows.

$$
P(Y_{5\lg{n}} \ge 1)  
\le
\frac{E \lbrack Y_{5\lg{n}}\rbrack}{1}
= 
n^{-4}.
$$

In other words, the probability that a given path in the quicksort
call tree has a depth that exceeds $5 \lg{n}$ is tiny.

From this bound, we can calculute a bound on the depth of the whole
tree.  Note first that the tree has exactly $n+1$ leaves because it
has $n$ internal nodes. Thus we have at most $n+1$ to consider.  The
probability that a given path expeeds a depth of $5\lg{n}$ is
$n^{-4}$. Thus, by [union
bound](https://en.wikipedia.org/wiki/Boole%27s_inequality) the
probability that any one of the paths exceed the depeth of $5\lg{n}$
is $(n+1) \cdot n^{-4} \le n^{-2}$.  Thus the probability that the
depth of the tree is greater than $5\lg{n}$ is $n^{-2}$.

By using [Total Expectation
Theorem](https://en.wikipedia.org/wiki/Law_of_total_expectation) or
the Law of total expectation, we can now calculate expected span by
dividing the sample space into mutually exclusive and exhaustive space
as follows.
 
$$
\begin{array}{ll}
E \lbrack S \rbrack 
& 
= 
E \lbrack S ~|~\mbox{Depth is no greater than}~5\lg{n} \rbrack 
\cdot
P(\mbox{Depth is no greater than}~5\lg{n})
+ 
E \lbrack S ~|~\mbox{Depth is greater than}~5\lg{n} \rbrack 
\cdot
P(\mbox{Depth is greater than}~5\lg{n})
\\
& 
\le
\lg^2{n} \cdot (1 - 1/n^{2}) + 
n^2 \cdot 1/n^{2} 
\\
& 
= 
O(lg^2{n})
\end{array}
$$

In thes bound, we used the fact that each call to quicksort has a span
of $O(\lg{n})$ because this is the span of `filter`.

Here is an alternative analysis.

Let $M_n = \max\{|L|, |R|\}$, which is the size of larger
subsequence. The span of quicksort is determined by the sizes of these
larger subsequences. For ease of analysis, we will assume that $|E| =
0$, as more equal elements will only decrease the span. As the
partition step uses `filter` we have the following recurrence for
span:

$$
S(n) = S(M_n) + O(\log n) 
$$

To develop some intuition for the span analysis, let's consider the
probability that we split the input sequence more or less evenly.  If
we select a pivot that is greater than $T_{n/4}$ and less than
$T_{3n/4}$ then $M_n$ is at most $3n/4$.  Since all keys are equally
likely to be selected as a pivot this probability is $\frac{3n/4 -
n/4}{n} = 1/2$.  The figure below illustrates this.

![Quicksort span intuition.](images/qsort-span-intuition.jpg)

This observations implies that at each level of the call tree (every
time a new pivot is selected), the size of the input to both calls
decrease by a constant fraction (of $4/3$).  At every two levels, the
probability that the input size decreases by $4/3$ is the probability
that it decreases at either step, which is at least $3/4$, etc.  Thus
at a small costant number of steps, the probability that we observe a
$4/3$ factor decrease in the size of the input approches $1$ quickly.
This suggest that at some after $c\log{n}$ levels quicksort should
complete.  We now make this intuition more precise.

For the analysis, we use the conditioning technique for computing
expectations as suggested by the total expectation theorem.  Let $X$
be a random variable and let $A_i$ be disjoint events that form a a
partition of the sample space such that $P(A_i) > 0$.  The [Total
Expectation
Theorem](https://en.wikipedia.org/wiki/Law_of_total_expectation) or
the Law of total expectation states that

$$
E[X] = \sum_{i=1}^{n}{P(A_i) \cdot E \lbrack X | A_i \rbrack}.
$$

Note first that $P(X_n \leq 3n/4) = 1/2$, since half of the randomly
chosen pivots results in the larger partition to be at most $3n/4$
elements: any pivot in the range $T_{n/4}$ to $T_{3n/4}$ will do,
where $T$ is the sorted input sequence.

By conditioning $S_n$ on the random variable $M_n$, we write,

$$
E \lbrack S_n \rbrack
= 
\sum_{m=n/2}^{n}{P(M_n=m) \cdot E \lbrack S_n | (M_n=m) \rbrack}.
$$
 
We can re-write this

$$
E \lbrack S_n \rbrack = \sum_{m=n/2}^{n}{P(M_n=m) \cdot E \lbrack S_m \rbrack}
$$

The rest is algebra.

$$
\begin{array}{ll}
E \lbrack S_n \rbrack 
& =  \sum_{m=n/2}^{n}{P(M_n=m) \cdot E \lbrack S_m \rbrack}
\\
& \leq P(M_n \leq \frac{3n}{4}) \cdot E \lbrack S_{\frac{3n}{4}} \rbrack + 
      P(M_n > \frac{3n}{4}) \cdot E \lbrack S_n \rbrack + c\cdot \log n
\\
& \leq \frac{1}{2} E \lbrack S_{\frac{3n}{4}} \rbrack + \frac{1}{2} E
\lbrack S_n \rbrack
\\
& \implies E \lbrack S_n \rbrack \leq E \lbrack S_{\frac{3n}{4}} \rbrack + 2c \log n.
\\
\end{array}
$$

This is a recursion in $E \lbrack S(\cdot) \rbrack$ and solves easily
to $E \lbrack S(n) \rbrack = O(\log^2 n)$.

### Observable Work Efficency and Scalability

For an implementation to be observably work efficient, we know that we
must control granularity by switching to a fast sequential sorting
algorithm when the input is small.  This is easy to achieve using our
granularity control technique by using `seqsort()`, a fast sequential
algorithm provided in the code base; `seqsort()` is really a call to
STL's sort function.  Of course, we have to assess observable work
efficiency experimentally after specifying the implementation.

The code for quicksort is shown below.  Note that we use our array
class `pparray` to store the input and output.  To partition the
input, we use our parallel `filter` function from the previous lecture
to parallelize the partitioning phase. Similarly, we use our parallel
concatenation function to constructed the sorted output.

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ {.cpp}
parray<int> quicksort(parray<int>& xs) {
  size_type n = xs.size();
  parray<int> result;
  spguard([&] { return n * std::log2(n); }, [&] {
    if (n == 0) {
      result = { };
    } else if (n == 1) {
      result = { xs[0] };
    } else {
      int p = xs[0];
      parray<parray<int>> partitions(3);
      partitions[0] = filter(xs.begin(), xs.end(), [&] (int x) { return x < p; });
      partitions[1] = filter(xs.begin(), xs.end(), [&] (int x) { return x == p; });
      partitions[2] = filter(xs.begin(), xs.end(), [&] (int x) { return x > p; });
      fork2([&] {
        partitions[0] = quicksort(partitions[0]);
      }, [&] {
        partitions[2] = quicksort(partitions[2]);
      });
      result = flatten(partitions);
    }
  }, [&] {
    result = seqsort(xs);
  });
  return result;
}
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

By using randomized-analysis techniques, it is possible to analyze the
work and span of this algorithm.  The techniques needed to do so are
beyond the scope of this book.  The interested reader can find more
details in [another book](http://www.parallel-algorithms-book.com/).

::::: {#ex-fact-randomized-quicksort .fact}

**Fact:** The randomized quicksort algorithm above has expected work
of $O(n \log n)$ and expected span of $O(\log^2 n)$, where $n$ is the
number of items in the input sequence.

:::::

One consequence of the work and span bounds that we have stated above
is that our quicksort algorithm is highly parallel: its average
parallelism is $\frac{O(n \log n)}{O(\log^2 n)} = \frac{n}{\log
n}$. When the input is large, there should be ample parallelism to
keep many processors well fed with work. For instance, when $n = 100$
million items, the average parallelism is $\frac{10^8}{\log 10^8}
\approx \frac{10^8}{40} \approx 3.7$ million. Since 3.7 million is
much larger than the number of processors in our machine, that is,
forty, we have a ample parallelism.

Unfortunately, the code that we wrote leaves much to be desired in
terms of observable work efficiency.  Consider the following
benchmarking runs that we performed on our 40-processor machine.

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
$ prun speedup -baseline "bench.baseline" -parallel "bench.opt -proc 1,10,20,30,40" -bench quicksort -n 100000000
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

The first two runs show that, on a single processor, our parallel
algorithm is roughly 6x slower than the sequential algorithm that we
are using as baseline! In other words, our quicksort appears to have
"6-observed work efficiency". That means we need at least six
processors working on the problem to see even a small improvement
compared to a good sequential baseline.

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
[1/6]
bench.baseline -bench quicksort -n 100000000
exectime 12.518
[2/6]
bench.opt -bench quicksort -n 100000000 -proc 1
exectime 78.960
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

The rest of the results confirm that it takes about ten processors to
see a little improvement and forty processors to see approximately a
2.5x speedup. <<sorting::quicksort-speedup, This plot>> shows the speedup plot
for this program. Clearly, it does not look good.

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
[3/6]
bench.opt -bench quicksort -n 100000000 -proc 10
exectime 9.807
[4/6]
bench.opt -bench quicksort -n 100000000 -proc 20
exectime 6.546
[5/6]
bench.opt -bench quicksort -n 100000000 -proc 30
exectime 5.531
[6/6]
bench.opt -bench quicksort -n 100000000 -proc 40
exectime 4.761
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

![Speedup plot for quicksort with $100000000$ elements.](images/quicksort-speedup.jpg)

Our analysis suggests that we have a good parallel algorithm for
quicksort, yet our observations suggest that, at least on our test
machine, our implementation is rather slow relative to our baseline
program. In particular, we noticed that our parallel quicksort started
out being 6x slower than the baseline algorithm. What could be to
blame?

In fact, there are many implementation details that could be to
blame. The problem we face is that identifying those causes
experimentally could take a lot of time and effort. Fortunately, our
quicksort code contains a few clues that will guide us in a good
direction.

It should be clear that our quicksort is copying a lot of data and,
moreover, that much of the copying could be avoided. The copying
operations that could be avoided, in particular, are the array copies
that are performed by each of the the three calls to filter and the
one call to concat. Each of these operations has to touch each item in
the input array.

Let us now consider a (mostly) in-place version of quicksort. This
code is mostly in place because the algorithm copies out the input
array in the beginning, but otherwise sorts in place on the result
array. The code for this algorithm appears just below.

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ {.cpp}
parray<int> in_place_quicksort(parray<int>& xs) {
  parray<int> result = copy(xs);  
  size_type n = xs.size();
  if (n == 0) {
    return result;
  }
  in_place_quicksort_rec(&result[0], n);
  return result;
}
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ {.cpp}
void in_place_quicksort_rec(int* A, size_type n) {
  if (n < 2) {
    return;
  }
  spguard([&] { return nlogn(n); }, [&] {
    int p = A[0];
    int* L = A;   // below L are less than pivot
    int* M = A;   // between L and M are equal to pivot
    int* R = A + n - 1; // above R are greater than pivot
    while (true) {
      while (! (p < *M)) {
        if (*M < p) {
          std::swap(*M,*(L++));
        }
        if (M >= R) {
          break;
        }
        M++;
      }
      while (p < *R) {
        R--;
      }
      if (M >= R) {
        break;
      }
      std::swap(*M,*R--);
      if (*M < p) {
        std::swap(*M,*(L++));
      }
      M++;
    }
    fork2([&] {
      in_place_quicksort_rec(A, L-A);
    }, [&] {
      in_place_quicksort_rec(M, A+n-M); // Exclude all elts that equal pivot
    });
  }, [&] {
    std::sort(A, A+n);
  });
}
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

We have good reason to believe that this code is, at least, going to
be more work efficient than our original solution. First, it avoids
the allocation and copying of intermediate arrays. And, second, it
performs the partitioning phase in a single pass. There is a catch,
however: in order to work mostly in place, our second quicksort code
sacrified on parallelism. In specific, observe that the partitioning
phase is now sequential. The span of this second quicksort is
therefore linear in the size of the input and its average parallelism
is therefore logarithmic in the size of the input.

::::: {#exercise-quicksort-span .exercise}

**Exercise:** Verify that the span of our second quicksort has linear
span and that the average parallelism is logarithmic.

:::::

So, we expect that the second quicksort is more work efficient but
should scale poorly. To test the first hypothesis, let us run the
second quicksort on a single processor.

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
$ bench.opt -bench in_place_quicksort -n 100000000 -proc 1
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Indeed, the running time of this code is essentially same as what we
observed for our baseline program.

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
exectime 12.500
total_idle_time 0.000
utilization 1.0000
result 1048575
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Now, let us see how well the second quicksort scales by performing
another speedup experiment.

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
$ prun speedup -baseline "bench.baseline" -parallel "bench.opt -proc 1,20,30,40" -bench quicksort,in_place_quicksort -n 100000000
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
[1/10]
bench.baseline -bench quicksort -n 100000000
exectime 12.031
[2/10]
bench.opt -bench quicksort -n 100000000 -proc 1
exectime 68.998
[3/10]
bench.opt -bench quicksort -n 100000000 -proc 20
exectime 5.968
[4/10]
bench.opt -bench quicksort -n 100000000 -proc 30
exectime 5.115
[5/10]
bench.opt -bench quicksort -n 100000000 -proc 40
exectime 4.871
[6/10]
bench.baseline -bench in_place_quicksort -n 100000000
exectime 12.028
[7/10]                             
bench.opt -bench in_place_quicksort -n 100000000 -proc 1
exectime 12.578
[8/10]
bench.opt -bench in_place_quicksort -n 100000000 -proc 20
exectime 1.731
[9/10]
bench.opt -bench in_place_quicksort -n 100000000 -proc 30
exectime 1.697
[10/10]
bench.opt -bench in_place_quicksort -n 100000000 -proc 40
exectime 1.661
Benchmark successful.  
Results written to results.txt.
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
$ pplot speedup -series bench
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

The <<sorting::quicksort-speedup-compare, plot below>> shows one
speedup curve for each of our two quicksort implementations. The
in-place quicksort is always faster. However, the in-place quicksort
starts slowing down a lot at 20 cores and stops after 30 cores. So, we
have one solution that is observably not work efficient and one that
is, and another that is the opposite. The question now is whether we
can find a happy middle ground.

![Speedup plot showing our quicksort and the in-place quicksort side
 by side. As before, we used $100000000$
 elements.](images/in-place-quicksort-speedup.jpg)

::::: {#exercise-quicksort-better .exercise}

**Exercise:** What can we do to write a better quicksort?

TIP: Eliminate unecessary copying and array allocations.

TIP: Eliminate redundant work by building the partition in one pass
instead of three.

TIP: Find a solution that has the same span as our first quicksort
code.

:::::

We encourage students to look for improvements to quicksort
independently. For now, we are going to consider parallel
mergesort. This time, we are going to focus more on achieving better
speedups.

Mergesort
---------

As a divide-and-conquer algorithm, the mergesort algorithm, is a good
candidate for parallelization, because the two recursive calls for
sorting the two halves of the input can be independent.  The final
merge operation, however, is typically performed sequentially.  It
turns out to be not too difficult to parallelize the merge operation
to obtain good work and span bounds for parallel mergesort. The
resulting algorithm turns out to be a good parallel algorithm,
delivering asymptotic, and observably work efficiency, as well as low
span.

::::: {#algorithm-mergesort .algorithm}

**Algorithm:** mergesort

- Divide the (unsorted) items in the input array into
two equally sized subrange.

- Recursively and in parallel sort each subrange.

- Merge the sorted subranges.

:::::

This process requires a "merge" routine which merges the contents of
two specified subranges of a given array. The merge routine assumes
that the two given subarrays are in ascending order. The result is the
combined contents of the items of the subranges, in ascending order.

The precise signature of the merge routine appears below and its
description follows. In mergesort, every pair of ranges that are
merged are adjacent in memory. This observation enables us to write
the following function. The function merges two ranges of source array
`xs`: `[lo, mid)` and `[mid, hi)`. A temporary array `tmp` is used as
scratch space by the merge operation. The function writes the result
from the temporary array back into the original range of the source
array: `[lo, hi)`.

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ {.cpp}
void merge(parray<int>& xs, parray<int>& tmp,
           size_type lo, size_type mid, size_type hi);
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ {.cpp}
parray<int> xs = {
  // first range: [0, 4)
  5, 10, 13, 14,
  // second range: [4, 9)
  1, 8,  10, 100, 101 };

parray<int> tmp(xs.size();

merge(xs, tmp, 0, 4, 9);

std::cout << "xs = " << xs << std::endl;
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
xs = { 1, 5, 8, 10, 10, 13, 14, 100, 101 }
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

To see why sequential merging does not work, let us implement the
merge function by using one provided by STL: `std::merge()`. This
merge implementation performs linear work and span in the number of
items being merged (i.e., `hi - lo`). In our code, we use this STL
implementation underneath the `merge()` interface that we described
just above.

Now, we can assess our parallel mergesort with a sequential merge, as
implemented by the code below.  The code uses the traditional
divide-and-conquer approach that we have seen several times already.

::::: {#exercise-mergesort-work-efficiency .exercise}

**Exercise:** Is the implementation asymptotically work efficient?

:::::

The code is asymptotically work efficient, because nothing significant
has changed between this parallel code and the serial code: just erase
the parallel annotations and we have a textbook sequential mergesort!

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ {.cpp}
parray<int> mergesort(parray<int>& xs) {
  size_type n = xs.size();
  parray<int> result(xs.begin(), xs.end());
  parray<int> tmp(n);
  mergesort_rec(result, tmp, 0, n);
}

void mergesort_rec(parray<int>& xs, parray<int>& tmp,
                   size_type lo, size_type hi) {
  size_type n = hi - lo;
  spguard([&] { return n * std::log2(n); }, [&] {
    if (n == 0) {
      // nothing to do
    } else if (n == 1) {
      tmp[lo] = xs[lo];
    } else {
      size_type mid = (lo + hi) / 2;
      fork2([&] {
        mergesort_rec(xs, tmp, lo, mid);
      }, [&] {
        mergesort_rec(xs, tmp, mid, hi);
      });
      merge(xs, tmp, lo, mid, hi);
    }
  }, [&] {
    if (hi-lo < 2) {
      return;
    }
    std::sort(&xs[lo], &xs[hi-1]+1);
  });
}
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

::::: {#exercise-mergesort-scale .exercise}

**Exercise:** How well does our "parallel" mergesort scale to multiple
processors, i.e., does it have a low span?

:::::

Unfortunately, this implementation has a large span: it is linear,
owing to the sequential merge operations after each pair of parallel
calls.  More precisely, we can write the work and span of this
implementation as follows:

::::: {#theorem-mergesort-bounds .theorem}

**Theorem:** Upper bounds on work and span of mergesort

$$
W(n) = 
\left\{
\begin{array}{ll}
1 & \mbox{if}~n \le 1 
\\
W(n/2) + W(n/2) + n 
\end{array}
\right.
$$

$$
S(n) = 
\left\{
\begin{array}{ll}
1 & \mbox{if}~n \le 1 
\\
\max (W(n/2),W(n/2)) + n 
\end{array}
\right.
$$

It is not difficult to show that these recursive equation solve to
$W(n) = \Theta (n\log{n})$ and $S(n) = \Theta (n)$.

:::::

With these work and span costs, the average parallelism of our
solution is $\frac{cn \log n}{2cn} = \frac{\log n}{2}$. Consider the
implication: if $n = 2^{30}$, then the average parallelism is
$\frac{\log 2^{30}}{2} = 15$. That is terrible, because it means that
the greatest speedup we can ever hope to achieve is 15x!

The analysis above suggests that, with sequential merging, our
parallel mergesort does not expose ample parallelism. Let us put that
prediction to the test. The following experiment considers this
algorithm on our 40-processor test machine. We are going to sort a
random sequence of 100 million items. The baseline sorting algorithm
is the same sequential sorting algorithm that we used for our
quicksort experiments: `std::sort()`.

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
$ prun speedup -baseline "bench.baseline" -parallel "bench.opt -proc 1,10,20,30,40" -bench mergesort_seqmerge -n 100000000
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

The first two runs suggest that our mergesort has better observable
work efficiency than our quicksort. The single-processor run of
parallel mergesort is roughly 50% slower than that of the sequential
baseline algorithm. Compare that to the 6x-slower running time for
single-processor parallel quicksort! We have a good start.

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
[1/6]
bench.baseline -bench mergesort_seqmerge -n 100000000
exectime 12.483
[2/6]
bench.opt -bench mergesort_seqmerge -n 100000000 -proc 1
exectime 19.407
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

The parallel runs are encouraging: we get 5x speedup with 40
processors.

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
[3/6]
bench.opt -bench mergesort_seqmerge -n 100000000 -proc 10
exectime 3.627
[4/6]
bench.opt -bench mergesort_seqmerge -n 100000000 -proc 20
exectime 2.840
[5/6]
bench.opt -bench mergesort_seqmerge -n 100000000 -proc 30
exectime 2.587
[6/6]
bench.opt -bench mergesort_seqmerge -n 100000000 -proc 40
exectime 2.436
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

But we can do better by using a parallel merge instead of a sequential
one: the speedup plot in <<mergesort-speedups>> shows three
speedup curves, one for each of three mergesort algorithms. The
`mergesort()` algorithm is the same mergesort routine that we have
seen here, except that we have replaced the sequential merge step by
our own parallel merge algorithm. The `cilksort()` algorithm is the
carefully optimized algorithm taken from the Cilk benchmark
suite. What this plot shows is, first, that the parallel merge
significantly improves performance, by at least a factor of two. The
second thing we can see is that the optimized Cilk algorithm is just a
little faster than the one we presented here. That's pretty good,
considering the simplicity of the code that we had to write.

![Speedup plot for three different implementations of mergesort using 100 million items.](images/mergesort-speedups.jpg){#image-better-speedups}

It turns out that we can do better by simply changing some of the
variables in our experiment. The plot shown in [a speedup
plot](#image-better-speedups) that we get when we change two
variables: the input size and the sizes of the items. In particular,
we are selecting a larger number of items, namely 250 million instead
of 100 million, in order to increase the amount of parallelism. And,
we are selecting a smaller type for the items, namely 32 bits instead
of 64 bits per item. The speedups in this new plot get closer to
linear, topping out at approximately 20x.

Practically speaking, the mergesort algorithm is memory bound because
the amount of memory used by mergesort and the amount of work
performed by mergesort are both approximately roughly linear. It is an
unfortunate reality of current multicore machines that the main
limiting factor for memory-bound algorithms is amount of parallelism
that can be achieved by the memory bus. The memory bus in our test
machine simply lacks the parallelism needed to match the parallelism
of the cores. The effect is clear after just a little experimentation
with mergesort. You can see this effect yourself, if you are
interested to change in the source code the type aliased by
`value_type`. For a sufficiently large input array, you should observe
a significant performance improvement by changing just the
representation of `value_type` from 64 to 32 bits, owing to the fact
that with 32-bit items is a greater amount of computation relative to
the number of memory transfers.

![Speedup plot for three different implementations of mergesort using 250 million items.](images/mergesort-speedups-250m-32bit.jpg)

::::: {#exercise-mergesort-scale .exercise}

**Exercise:** stable mergesort

An important property of the sequential merge-sort algorithm is that
it is stable: it can be written in such a way that it preserves the
relative order of equal elements in the input.  Is the parallel
merge-sort algorithm that you designed stable?  If not, then can you
find a way to make it stable?  

:::::