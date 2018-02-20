Experimenting with SPTL
=======================

We are now going to study the practical performance of our parallel
algorithms written with SPTL on multicore computers.

To be concrete with our instructions, we assume that our username is
`sptl` and that our home directory is `/home/sptl/`. You need to
replace these settings with your own where appropriate.

Obtain source files
-------------------

Let's start by downloading the SPTL sources.  The SPTL sources that we
are going to use are part of a branch that we created specifically for
this course. You can access the sources either via the tarball linked
by the [github
webpage](https://github.com/deepsea-inria/sptl/tree/edu) or, if you
have `git`, via the command below.

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
$ cd /home/sptl
$ git clone -b edu https://github.com/deepsea-inria/sptl.git
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Software Setup
--------------

You can skip this section if you are using a computer already setup by
us or you have installed an image file containing our software.  To
skip this part and use installed binaries, see the heading "Starting
with installed binaries", [linked here](#sec-starting-with-binaries).

### Check for software dependencies

Currently, the software associated with this course supports Linux
only. Any machine that is configured with a recent version of Linux
and has access to at least two processors should be fine for the
purposes of this course. Before we can get started, however, the
following packages need to be installed on your system.

-------------------------------------------------------------------------------
 Software dependency            Version      Nature of dependency
------------------------------- ------------ ----------------------------------
[gcc](https://gcc.gnu.org/)     >= 4.9.0     required to build SPTL binaries

[ocaml](https://ocaml.org/)     >= 4.0.0     required to build the benchmarking
                                             tools (i.e., pbench and pview)
                                             
[R](http://www.r-project.org/)  >= 2.4.1     required by benchmarking tools to
                                             generate reports in bar plot and
                                             scatter plot form
                                             
latex                           recent       required by benchmarking tools to
                                             generate reports in tabular form
                                             
[git](http://git-scm.com/)      recent       can be used to access SPTL source
                                             files
-------------------------------------------------------------------------------

Table: Software dependencies.

The rest of this section explains what are the optional software
dependencies and how to configure SPTL to use them. We are going to
assume that all of these software dependencies have been installed in
the folder `/home/sptl/Installs/`.

::::: {#todo-software .todo}

TODO

:::::

Starting with installed binaries {#sec-starting-with-binaries}
--------------------------------

At this point, you have either installed all the necessary software to
work with SPTL or these are installed for you.  In either case, make
sure that your `PATH` variable makes the software visible.  For
setting up your `PATH` variable on andrew.cmu domain, see below.

### Specific set up for the andrew.cmu domain

We have installed much of the needed software on andrew.cmu.edu.  So
you need to go through a relatively minimal set up.

First set up your `PATH` variable to refer to the right directories.
Using cshell

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
setenv PATH  /opt/rh/devtoolset-3/root/usr/bin:/usr/lib64/qt-3.3/bin:/usr/lib64/ccache:/usr/local/bin:/bin:/usr/bin:./
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

The part added to the default PATH on andrew is 

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
/opt/rh/devtoolset-3/root/usr/bin
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

It is important that this is at the beginning of the `PATH` variable.
To make interaction easier, we also added the relative path `./` to
the `PATH` variable.

### Fetch the benchmarking tools

We are going to use two command-line tools to help us to run
experiments and to analyze the data. These tools are part of a library
that we developed, which is named pbench. The pbench sources are
available via github.

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
$ cd /home/sptl
$ git clone https://github.com/deepsea-inria/pbench.git
$ git clone https://github.com/deepsea-inria/pview.git
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

### Build the tools

The following command builds the tools, namely `prun` and `pplot`. The
former handles the collection of data and the latter the
human-readable output (e.g., plots, tables, etc.).

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
$ make -C /home/sptl/pbench/
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Make sure that the build succeeded by checking the pbench directory
for the files `prun` and `pplot`. If these files do not appear, then
the build failed.

### Create aliases

We recommend creating the following aliases.

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
$ alias prun '/home/sptl/pbench/prun'
$ alias pplot '/home/sptl/pbench/pplot'
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

It will be convenient for you to make these aliases persistent, so
that next time you log in, the aliases will be set. Add the commands
above to your shell configuration file.

### Visualizer Tool

When we are tuning our parallel algorithms, it can be helpful to
visualize their processor utilization over time, just in case there
are patterns that help to assign blame to certain regions of
code. Later, we are going to use the utilization visualizer that comes
packaged along with SPTL. To build the tool, run the following make
command.

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
$ make -C /home/sptl/pview pview
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Let us create an alias for the tool.

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
$ alias pview '/home/sptl/pview/pview'
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

We recommend that you make this alias persistent by putting it into
your shell configuration file (as you did above for the pbench tools).

### Using the Makefile 

SPTL comes equipped with a `Makefile` that can generate several
different kinds of executables. These different kinds of executables
and how they can be generated is described below for a benchmark
program `pgm`.

- ***baseline***: build the baseline with command `make pgm.baseline`
- ***elision***: build the sequential elision with command `make pgm.elision`
- ***optimized***: build the optimized binary with command `make pgm.opt`
- ***log***: build the log binary with command `make pgm.log`
- ***debug***: build the debug binary with the command `make pgm.dbg`

To speed up the build process, add to the `make` command the option
`-j` (e.g., `make -j pgm.opt`). This option enables `make` to
parallelize the build process. Note that, if the build fails, the
error messages that are printed to the terminal may be somewhat
garbled. As such, it is better to use `-j` only if after the debugging
process is complete.

Task 1: Run the baseline Fibonacci
----------------------------------

We are going to start our experimentation with the parallel Fibonacci
program. We first build the baseline version.

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
$ cd /home/sptl/book/code
$ make fib.baseline
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

::::: {#warning-command-line .warning}

**Warning:** The command-line examples that we show here assume that
you have `.` in your `$PATH`. If not, you may need to prefix
command-line calls to binaries with `./` (e.g., `./fib.baseline`).

:::::

The file extension `.baseline` means that binary is going to use the
sequential-baseline version of the program.

We can now experiment with the baseline of the Fibonacci program. To
have the program compute the $n^{th}$ Fibonacci number, pass the
command-line argument `-n`.

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
$ fib.baseline -n 40
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

On our machine, the output of this run is the following.

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
exectime 0.405
result  102334155
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

The two lines above provide useful information about the run.

- The `exectime` indicates the wall-clock time in seconds that is
taken by the benchmark. In general, this time measures only the time
taken by the benchmark under consideration. It does not include the
time taken to generate the input data, for example.

- The `result` field reports a value computed by the benchmark. In
this case, the value is the $40^{th}$ Fibonacci number.

Task 2: Run the sequential elision of Fibonacci
-----------------------------------------------

The `.elision` extension means that parallel algorithms (not
sequential baseline algorithms) are compiled. However, all instances
of `fork2()` are erased as described in an <<ch:fork-join, earlier chapter>>.

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
$ make fib.elision
$ fib.elision -n 40
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

The run time of the sequential elision in this case is similar to the
run time of the sequential baseline because the two are similar
codes. However, for most other algorithms, the baseline will typically
be at least a little faster.

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
exectime 0.429
result  102334155
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Task 3: Run parallel Fibonacci
------------------------------

The `.opt` extension means that the program is compiled with full
support for parallel execution. Unless specified otherwise, however,
the parallel binary uses just one processor.

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
$ make fib.opt
$ fib.opt -n 40
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

The output of this program is similar to the output of the previous
two programs.

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
exectime 0.404
result  102334155
nb_steals       0
total_idle_time 0.000000
launch_duration 0.403734
utilization     1.000000
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Because our machine has 8 processors, we can run the same application
using all available processors.  Before running this command, please
adjust the `-proc` option to match the number of cores that your
machine has.  Note that you can use any number of cores up to the
number you have available.  You can use `nproc` or `lscpu` to
determine the number of cores your machine has.

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
$ fib.opt -n 40 -proc 8
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

We see from the output of the 8-processor run that our program ran
faster than the sequential runs. Moreover, the `utilization` field
tells us that approximately 98% of the total time spent by the 8
processors was spent performing useful work, not idling.

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
exectime 0.065
result  102334155
nb_steals       68
total_idle_time 0.006619
launch_duration 0.065512
utilization     0.987371
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

::::: {#warning-sptl-proc .warning}

**Warning:** SPTL allows the user to select the number of processors
by the `-proc` key. The maximum value for this key is the number of
processors that are available on the machine. SPTL raises an error if
the programmer asks for more processors than are available.

:::::

Measuring performance by speedup
--------------------------------

We may ask at this point: What is the improvement that we just
observed from the parallel run of our program? One common way to
answer this question is to measure the "speedup".

::::: {#definition-speedup .definition}

**Definition:** $P$-processor speedup

The speedup on $P$ processors is the ratio $T_B/T_P$, where the term
$T_B$ represents the run time of the sequential baseline program and
the term $T_P$ the time measured for the $P$-processor run.

:::::

::::: {#note-baseline .note}

*Note:* The importance of selecting a good baseline

Note that speedup is defined with respect to a baseline program.  How
exactly should this baseline program be chosen?  One option is to take
the sequential elision as a baseline. The speedup curve with such a
baseline can be helpful in determining the scalability of a parallel
algorithm but it can also be misleading, especially if speedups are
taken as a indicator of good performance, which they are not because
they are only relative to a specific baseline.  For speedups to be a
valid indication of good performance, they must be calculated against
an optimized implementation of the best serial algorithm (for the same
problem.)

:::::

The speedup at a given number of processors is a good starting point
on the way to evaluating the scalability of the implementation of a
parallel algorithm. The next step typically involves considering
speedups taken from varying numbers of processors available to the
program. The data collected from such a speedup experiment yields a
***speedup curve***, which is a curve that plots the trend of the
speedup as the number of processors increases. The shape of the
speedup curve provides valuable clues for performance and possibly for
tuning: a flattening curve suggests lack of parallelism; a curve that
arcs up and then downward suggests that processors may be wasting time
by accessing a shared resource in an inefficient manner (e.g., false
sharing); a speedup curve with a constant slope indicates at least
some scaling.


::::: {#ex-speedup-fib .example}

**Example:** Speedup for our run of Fibonacci on 40 processors

The speedup $T_B/T_{8}$ equals $0.405/0.065 = 6.2$x.  Although not
linear (i.e., 8x), this speedup is decent considering factors such as:
the capabilities of our machine; the overheads relating to
parallelism; and the small size of the problem compared to the
computing power that our machine offers.

:::::

Generate a speedup plot
-----------------------

Let us see what a speedup curve can tell us about our parallel
Fibonacci program. We need to first get some data. The following
command performs a sequence of runs of the Fibonacci program for
varying numbers of processors. You can now run the command yourself.

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
$ prun speedup -baseline "fib.baseline" -parallel "fib.opt -proc 1,2,4,6,8" -n 40
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Run the following command to generate the speedup plot.

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
$ pplot speedup
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

If successful, the command generates a file named `plots.pdf`. The
output should look something like the [plot below](#fib-speedup-plot)

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
Starting to generate 1 charts.
Produced file plots.pdf.
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

![Speedup curve for the computation of the $40^{th}$ Fibonacci
 number.](images/speedup-plot-fib-1.jpg){#fib-speedup-plot}

The plot shows that our Fibonacci application scales well, up to about
twenty processors. As expected, at twenty processors, the curve dips
downward somewhat. We know that the problem size is the primary factor
leading to this dip. How much does the problem size matter?  The
speedup plot in the [Figure below](#fib-weak-scaling-plot) shows
clearly the trend. As our problem size grows, so does the speedup
improve, until at the calculation of the $45^{th}$ Fibonacci number,
the speedup curve is close to being linear.

![Speedup plot showing speedup curves at different problem
 sizes.](images/fib-weak-scaling.jpg){#fib-weak-scaling-plot}

::::: {#note-prun1 .note}

**Note:** The `prun` and `pplot` tools have many more features than
those demonstrated here. For details, see the documentation provided
with the tools in the file named `README.md`.

:::::

::::: {#warning-noise .warning}

**Noise in experiments**

The run time that a given parallel program takes to solve the same
problem can vary noticeably because of certain effects that are not
under our control, such as OS scheduling, cache effects, paging,
etc. We can consider such noise in our experiments random noise. Noise
can be a problem for us because noise can lead us to make incorrect
conclusions when, say, comparing the performance of two algorithms
that perform roughly the same. To deal with randomness, we can perform
multiple runs for each data point that we want to measure and consider
the mean over these runs. The `prun` tool enables taking multiple runs
via the `-runs` argument. Moreover, the `pplot` tool by default shows
mean values for any given set of runs and optionally shows error
bars. The documentation for these tools gives more detail on how to
use the statistics-related features.

:::::

Superlinear speedup
-------------------

Suppose that, on our 8-processor machine, the speedup that we observe
is larger than 8x. It might sound improbable or even impossible. But
it can happen. Ordinary circumstances should preclude such a
*superlinear speedup*, because, after all, we have only forty
processors helping to speed up the computation. Superlinear speedups
often indicate that the sequential baseline program is suboptimal.
This situation is easy to check: just compare its run time with that
of the sequential elision. If the sequential elision is faster, then
the baseline is suboptimal. Other factors can cause superlinear
speedup: sometimes parallel programs running on multiple processors
with private caches benefit from the larger cache capacity. These
issues are, however, outside the scope of this course. As a rule of
thumb, superlinear speedups should be regarded with suspicion and the
cause should be investigated.


Visualize processor utilization
-------------------------------

The 6x speedup that we just calculated for our Fibonacci benchmark was
a little dissapointing, and the 86% processor utilization of the run
left 14% utilization for improvement.  We should be suspicious that,
although seemingly large, the problem size that we chose, that is, $n
= 40$, was probably a little too small to yield enough work to keep
all the processors well fed. To put this hunch to the test, let us
examine the utilization of the processors in our system. We need to
first build a binary that collects and outputs logging data.

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
$ make fib.log
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

We run the program with the new binary in the same fashion as before.

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
$ fib.log -proc 8 -n 40
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

The output looks something like the following.

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
exectime 0.019
launch_duration 0.019
utilization     0.8639
thread_send     205
thread_exec     4258
thread_alloc    2838
utilization 0.8639
result 63245986
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

We need to explain what the new fields mean.

- The `thread_send` field tells us that 233 threads were exchaged
between processors for the purpose of load balancing;
- the `thread_exec` field that 5179 threads were executed by the scheduler;
- the `thread_alloc` field that 3452 threads were freshly allocated.

Each of these fields can be useful for tracking down
inefficiencies. The number of freshly allocated threads can be a
strong indicator because in C++ thread allocation costs can sometimes
add up to a significant cost. In the present case, however, none of
the new values shown above are highly suspicious, considering that
there are all at most in the thousands.

Since we have not yet found the problem, let us look at the
visualization of the processor utilization using our `pview` tool. To
get the necessary logging data, we need to run our program again, this
time passing the argument `--pview`.

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
$ fib.log -n 40 -proc 8 --pview
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

When the run completes, a binary log file named `LOG_BIN` should be
generated in the current directory. Every time we run with `--pview`
this binary file is overwritten. To see the visualization of the log
data, we call the visualizer tool from the same directory.

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
$ pview
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

The output we see on our 8-processor machine is shown in the [Figure
below](#utilization-plot-fib). The window shows one bar per
processor. Time goes from left to right. Idle time is represented by
red and time spent busy with work by grey.  You can zoom in any part
of the plot by clicking on the region with the mouse.  To reset to the
original plot, press the space bar.  From the visualization, we can
see that most of the time, particularly in the middle, all of the
processors keep busy. However, there is a lot of idle time in the
beginning and end of the run.  This pattern suggests that there just
is not enough parallelism in the early and late stages of our
Fibonacci computation.

![Utilization plot for computation of $40^{th}$ Fibonacci
 number](images/fib-39-utilization.jpg){#utilization-plot-fib}

Strong versus weak scaling
--------------------------

We are pretty sure that or Fibonacci program is not scaling as well is
it could. But poor scaling on one particular input for $n$ does not
necessarily mean there is a problem with the scalability our parallel
Fibonacci program in general. What is important is to know more
precisely what it is that we want our Fibonacci program to achieve.
To this end, let us consider a distinction that is important in
high-performance computing: the distinction between strong and weak
scaling. So far, we have been studying the strong-scaling profile of
the computation of the $40^{th}$ Fibonacci number. In general, strong
scaling concerns how the run time varies with the number of processors
for a fixed problem size. Sometimes strong scaling is either too
ambitious, owing to hardware limitations, or not necessary, because
the programmer is happy to live with a looser notion of scaling,
namely weak scaling. In weak scaling, the programmer considers a
fixed-size problem per processor. We are going to consider something
similar to weak scaling. In the [Figure below](#fib-utilization-by-n),
we have a plot showing how processor utilization varies with the input
size. The situation dramatically improves from 12% idle time for the
$40^{th}$ Fibonacci number down to 5% idle time for the $41^{st}$ and
finally to 1% for the $45^{th}$. At just 1% idle time, the utilization
is excellent.

![How processor utilization of Fibonacci computation varies with input size.](images/fib-utilization-by-n.jpg){#fib-utilization-by-n}

The scenario that we just observed is typical of multicore
systems. For computations that perform relatively little work, such as
the computation of the $40^{th}$ Fibonacci number, properties that are
specific to the hardware, OS, and SPTL load-balancing algorithm can
noticeably limit processor utilization. For computations that perform
lots of highly parallel work, such limitations are barely noticeable,
because processors spend most of their time performing useful
work. Let us return to the largest Fibonacci instance that we
considered, namely the computation of the $45^{th}$ Fibonacci number,
and consider its utilization plot.

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
$ bench.log -bench fib -n 45 -proc 40 --pview
$ pview
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

The utilization plot is shown in the [Figure
below](#fib-utilization-45). Compared the to utilization plot we saw
in the [Figure above for n=40](#utilization-plot-fib), the red regions
are much less prominent overall and the idle regions at the beginning
and end are barely noticeable.

![Utilization plot for computation of 45th Fibonacci
 number.](images/fib-45-utilization.jpg){#fib-utilization-45}


Summary
-------

We have seen in this lab how to build, run, and evaluate our parallel
programs. Concepts that we have seen, such as speedup curves, are
going to be useful for evaluating the scalability of our future
solutions. Strong scaling is the gold standard for a parallel
implementation. But as we have seen, weak scaling is a more realistic
target in most cases.

