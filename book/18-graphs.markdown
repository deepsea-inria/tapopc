Graphs
======

In recent years, a great deal of interest has grown for frameworks
that can process very large graphs. Interest comes from a diverse
collection of fields. To name a few: physicists use graph frameworks
to simulate emergent properties from large networks of particles;
companies such as Google mine the web for the purpose of web search;
social scientists test theories regarding the origins of social
trends.

In response, many graph-processing frameworks have been implemented
both in academia and in the industry. Such frameworks offer to client
programs a particular application programming interface. The purpose
of the interface is to give the client programmer a high-level view of
the basic operations of graph processing. Internally, at a lower level
of abstraction, the framework provides key algorithms to perform basic
functions, such as one or more functions that "drive" the traversal of
a given graph.

The exact interface and the underlying algorithms vary from one
graph-processing framework to another. One commonality among the
frameworks is that it is crucial to harness parallelism, because
interesting graphs are often huge, making it practically infeasible to
perform sequentially interesting computations.

Graph representation
--------------------

We will use an adjacency lists representation based on ***compressed
arrays*** to represent directed graphs.  In this representation, a
graph is stored as a compact array containing the neighbors of each
vertex. Each vertex in the graph $G = (V, E)$ is assigned an integer
identifier $v \in \{ 0, \ldots, n-1 \}$, where $n = |V|$ is the number
of vertices in the graph.  The representation then consists of two
array.

- The ***edge array*** contains the adjacency lists of all vertices
  ordered by the vertex ids.

- The ***vertex array*** stores an index for each vertex that
  indicates the starting position of the adjacency list for that
  vertex in the edge array. This array implements the

![A graph (top) and its compressed-array representation (bottom)
consisting of the vertex and the edge arrays.  The sentinel value "-1"
is used to indicate a non-vertex id.](images/dir-graph-1.jpg)

![The compressed-array representation.](images/compressed-array.jpg)

The compressed-array representation requires a total of $n + m$
vertex-id cells in memory, where $n = |V|$ and $m = |E|$. Furthermore,
it involves very little indirection, making it possible to perform
many interesting graph operations efficiently.  For example, we can
determine the out-neighbors af a given vertex with constant work.
Similarly, we can determine the out-degree of a given vertex with
constant work.

::::: {#exercise-graph-outdegree .exercise}

**Exercise:** Give a constant-work algorithm for computing the
  out-degree of a vertex.

:::::

Space use is a major concern because graphs can have tens of billions
of edges or more. The Facebook social network graph (including just
the network and no metadata) uses 100 billion edges, for example, and
as such could fit snugly into a machine with 2TB of memory. Such a
large graph is a greater than the capacity of the RAM available on
current personal computers. But it is not that far off, and there are
many other interesting graphs that easily fit into just a few
gigabytes. For simplicity, we always use 64 bits to represent vertex
identifiers; for small graphs 32-bit representation can work just as
well.

We implemented the adjacency-list representation based on compressed
arrays with a class called `adjlist`.

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ {.cpp}
using vtxid_type = size_type;
using neighbor_list = const size_type*;

class adjlist {
public:
  size_type get_nb_vertices() const;
  size_type get_nb_edges() const;  
  size_type get_out_degree_of(vtxid_type v) const;
  neighbor_list get_out_edges_of(vtxid_type v) const;
};
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

::::: {#ex-graph-creation .example}

**Example:** graph creation

Sometimes it is useful for testing and debugging purposes to create a
graph from a handwritten example. For this purpose, we define a type
to express an edge. The type is a pair type where the first component
of the pair represents the source and the second the destination
vertex, respectively.

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ {.cpp}
using edge_type = std::pair<vtxid_type, vtxid_type>;
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

In order to create an edge, we use the following function, which takes
a source and a destination vertex and returns the corresponding edge.

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ {.cpp}
edge_type mk_edge(vtxid_type source, vtxid_type dest) {
  return std::make_pair(source, dest);
}
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Now, specifying a (small) graph in textual format is as easy as
specifying an edge list. Moreover, getting a textual representation of
the graph is as easy as printing the graph by `cout`.

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ {.cpp}
adjlist graph = { mk_edge(0, 1), mk_edge(0, 3), mk_edge(5, 1), mk_edge(3, 0) };
std::cout << graph << std::endl;
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Output:

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
digraph {
0 -> 1;
0 -> 3;
3 -> 0;
5 -> 1;
}
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

![Sample graph.](images/sample-graph.jpg)

:::::

::::: {#note-type-operator .note}

*Note:* The output above is an instance of the "dot" format. This
format is used by a well-known graph-visualization tool called
[graphviz](http://www.graphviz.org/). The diagram below shows the
visualization of our example graph that is output by the graphviz
tool.  You can easily generate such visualizations for your graphs by
using online tools, such [this
one](http://sandbox.kidstrythisathome.com/erdos/).

:::::

::::: {#ex-adjacency-list-interface .example}

**Example:** Adjacency-list interface

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ {.cpp}
adjlist graph = { mk_edge(0, 1), mk_edge(0, 3), mk_edge(5, 1), mk_edge(3, 0),
                  mk_edge(3, 5), mk_edge(3, 2), mk_edge(5, 3) };
std::cout << "nb_vertices = " << graph.get_nb_vertices() << std::endl;
std::cout << "nb_edges = " << graph.get_nb_edges() << std::endl;
std::cout << "neighbors of vertex 3:" << std::endl;
neighbor_list neighbors_of_3 = graph.get_out_edges_of(3);
for (size_type i = 0; i < graph.get_out_degree_of(3); i++)
  std::cout << " " << neighbors_of_3[i];
std::cout << std::endl;
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Output:

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
nb_vertices = 6
nb_edges = 7
neighbors of vertex 3:
 0 5 2
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

:::::

Next, we are going to study a version of breadth-first search that is
useful for searching in large in-memory graphs in parallel. After
seeing the basic pattern of BFS, we are going to generalize a little
to consider general-purpose graph-traversal techniques that are useful
for implementing a large class of parallel graph algorithms.

Breadth-first search
--------------------

The breadth-first algorithm is a particular graph-search algorithm
that can be applied to solve a variety of problems such as finding all
the vertices reachable from a given vertex, finding if an undirected
graph is connected, finding (in an unweighted graph) the shortest path
from a given vertex to all other vertices, determining if a graph is
bipartite, bounding the diameter of an undirected graph, partitioning
graphs, and as a subroutine for finding the maximum flow in a flow
network (using Ford-Fulkerson's algorithm).  As with the other graph
searches, BFS can be applied to both directed and undirected graphs.

The idea of ***breadth first search***, or ***BFS*** for short, is to
start at a _source_ vertex $s$ and explore the graph outward in all
directions level by level, first visiting all vertices that are the
(out-)neighbors of $s$ (i.e. have distance 1 from $s$), then vertices
that have distance two from $s$, then distance three, etc.  More
precisely, suppose that we are given a graph $G$ and a source $s$.  We
define the ***level*** of a vertex $v$ as the shortest distance from
$s$ to $v$, that is the number of edges on the shortest path
connecting $s$ to $v$.

::::: {#ex-bfs-levels .example}

**Example:** BFS Levels

A graph, where each vertex is labeled with its level.

![A graph and its levels.](images/directed-graph-levels.jpg)
				   
:::::

At a high level, BFS algorithm maintains a set of vertices called
`visited`, which contain the vertices that have been visited, and a
set of vertices called `frontier`, which contain the vertices that are
not visited but that are adjacent to a visited vertex.  It then visits
a vertex in the frontier and adds its out-neighbors to the frontier.

### Sequential BFS

Many variations of BFS have been proposed over the years. The one that
may be most widely known is the classic sequential BFS that uses a
FIFO queue to represent the `frontier`. The `visited` set can be
represented as some array data structure, or can be represented
implicitly by keeping a flat at each vertex that indicating whether
the vertex is visited or not.


Pseudocode for serial BFS 

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void sequential_bfs (G = (V,E), s)

frontier = <s>
visited = {}

while frontier is not <> do
  let <v,frontier_n> be frontier
  if v is not in visited then
    visit v
    foreach out-neighbor u of v do
      frontier_n = append frontier_n <u>     
    frontier = frontier_n
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

### Parallel BFS

Our goal is to design and implement a parallel algorithm for BFS that
is observably work efficient and has plenty of parallelism.  There is
natural parallelism in BFS because the vertices in each level can
actually be visited in parallel, as shown in the pseudo-code below.

Pseudo-code for parallel BFS

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
frontier = {source}
visited = {}
level = 0
while frontier is not {} do  
    next = {}
    let {v_1, ..., v_m} be frontier
    parallel for i = 1 to m do
      visit v_i
    visited = visited set-union frontier

    next = out_neighbors(v_1) set-union ... set-union  out_neighbors(v_m) 
    frontier = next set-difference visited
    level = level + 1
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

::::: {#exercise-bfs-levels .exercise}

**Exercise:** Convince yourself that this algorithm does indeed
perform a BFS by performing a level-by-level traversal.

:::::
 
Note that we can also compute the next set (frontier) in parallel by
performing a reduce with the set-union operation, and then by taking a
set-difference operation.

Our goal is to implement an observably work-efficient version of this
algorithm on a hardware-shared memory parallel machine such as a
modern multicore computer.  The key challenge is implementing the set
operations on the `visited`, `frontier`, and `next` sets.  Apart from
maintaining a visited map to prevent a vertex from being visited more
than once, the serial algorithm does not have to perform these
operations.

To achieve work efficiently, we will use atomic read-modify-write
operations, specifically compare-and-swap, to mark visited vertices
and use an array representation for the frontier.  To achieve
observable work efficiency, we will change the notion of the frontier
slightly.  Instead of holding the vertices that we are will visit
next, the frontier will hold the vertices we just visited.  At each
level, we will visit the neighbors of the vertices in the frontier,
but only if they have not yet been visited. This guard is necessary,
because two vertices in the frontier can both have the same vertex $v$
as their neighbor, causing $v$ to be visited multiple times.  After we
visit all the neighbors of the vertices in the frontier at this level,
we assign the frontier to be the vertices visited.  The pseudocode for
this algorithm is shown below.


Pseudocode for parallel BFS

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
level = 0
parallel for i = 1 to n do 
  visited[i] = false

visit source
visited[source] = true
frontier = {source}

while frontier is not {} do
   level = level + 1
   let {v_1, ..., v_m} be frontier
   parallel for i = 1 to m do
       let {u_1, ..., u_l} be out-neighbors(v_i)
       parallel for j = 1 to l do
         if compare_and_swap (&visited[u_j], false, true) succeeds then
           visit u_j
       frontier = vertices visited at this level
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

As show in the pseudocode, there are at least two clear opportunities
for parallelism. The first is the parallel loop that processes the
frontier and the second the parallel for loop that processes the
neighbors of a vertex. These two loops should expose a lot of
parallelism, at least for certain classes of graphs.  The outer loop
exposes parallelism when the frontier gets to be large. The inner loop
exposes parallelism when the traversal reaches a vertex that has a
high out degree.

To keep track of the visited vertices, the pseudo-code use an array of
booleans `visited[v]` of size $n$ that is keyed by the vertex
identifier. If `visited[v] == true`, then vertex `v` has been visited
already and has not otherwise.  We used an atomic compare-and-swap
operation to update the visited array because otherwise vertices can
be visited multiple times.  To see this suppose now that two
processors, namely $A$ and $B$, concurrently attempt to visit the same
vertex `v` (via two different neighbors of `v`) but without the use of
atomic operations.  If $A$ and $B$ both read `visited[v]` at the same
time, then both consider that they can visit `v`. Both processors then
mark `v` as visited and then proceed to visit the neighbors of `v`. As
such, `v` will be visited twice and subsequently have its outgoing
neighbors processed twice.

::::: {#exercise-bfs-races .exercise}

**Exercise:** Clearly, the race conditions on the visited array that
we described above can cause BFS to visit any given vertex twice.

- Could such race conditions cause the BFS to visit some vertex
  that is not reachable? Why or why not?

- Could such race conditions cause the BFS to not visit some vertex
  that is reachable? Why or why not?

- Could such race conditions trigger infinite loops? Why or why not?

:::::

In the rest of this section, we describe more precisely how to
implement the parallel BFS algorithm in C++.

In C++, we can we can use lightweight atomic memory, as described in
[this Chapter](#mutual-exclusion) to eliminate race conditions. The
basic idea is to guard each cell in our "visited" array by an atomic
type.

::::: {#ex-accessing-atomic-cell .example}

**Example:** accessing the contents of atomic memory cells

Access to the contents of any given cell is achieved by the `load()`
and `store()` methods.

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ {.cpp}
const size_type n = 3;
std::atomic<bool> visited[n];
size_type v = 2;
visited[v].store(false);
std::cout << visited[v].load() << std::endl;
visited[v].store(true);
std::cout << visited[v].load() << std::endl;
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Output:

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
0
1
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

:::::

The key operation that enables us to eliminate the race condition is
the ***compare and exchange operation***.  This operation performs the
following steps, atomically:

- Read the contents of the target cell in the visited array.
- If the contents is false (i.e., equals the contents of `orig`), then
  write `true` into the cell and return `true`.
- Otherwise, just return `false`.

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ {.cpp}
const size_type n = 3;
std::atomic<bool> visited[n];
size_type v = 2;
visited[v].store(false);
bool orig = false;
bool was_successful = visited[v].compare_exchange_strong(orig, true);
std::cout << "was_successful = " << was_successful << "; 
visited[v] = " << visited[v].load() << std::endl;
bool orig2 = false;
bool was_successful2 = visited[v].compare_exchange_strong(orig2, true);
std::cout << "was_successful2 = " << was_successful2 << "; 
visited[v] = " << visited[v].load() << std::endl;
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Output:

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
was_successful = 1; visited[v] = 1
was_successful2 = 0; visited[v] = 1
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

So far, we have seen pseudocode that describes at a high level the
idea behind the parallel BFS. We have seen that special care is
required to eliminate problematic race conditions.  Let's now put
these ideas together to complete and implementation.  The following
function signature is the signature for our parallel BFS
implementation. The function takes as parameters a graph and the
identifier of a source vertex and returns an array of boolean flags.

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ {.cpp}
parray<bool> bfs(const adjlist& graph, vtxid_type source);
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

The flags array is a length $|V|$ array that specifies the set of
vertices in the graph which are reachable from the source vertex: a
vertex with identifier `v` is reachable from the given source vertex
if and only if there is a `true` value in the $v^{\mathrm{th}}$
position of the flags array that is returned by `bfs`.

::::: {#ex-parallel-bfs .example}

**Example:** Parallel BFS

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ {.cpp}
adjlist graph = { mk_edge(0, 1), mk_edge(0, 3), mk_edge(5, 1), mk_edge(3, 0),
                  mk_edge(3, 5), mk_edge(3, 2), mk_edge(5, 3),
                  mk_edge(4, 6), mk_edge(6, 2) };
std::cout << graph << std::endl;
sparray reachable_from_0 = bfs(graph, 0);
std::cout << "reachable from 0: " << reachable_from_0 << std::endl;
sparray reachable_from_4 = bfs(graph, 4);
std::cout << "reachable from 4: " << reachable_from_4 << std::endl;
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

The following diagram shows the structure represented by `graph`. 

![Graph from the example.](images/sample-graph-2.jpg)

Output:

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
digraph {
0 -> 1;
0 -> 3;
3 -> 0;
3 -> 5;
3 -> 2;
4 -> 6;
5 -> 1;
5 -> 3;
6 -> 2;
}
reachable from 0: { 1, 1, 1, 1, 0, 1, 0 }
reachable from 4: { 0, 0, 1, 0, 1, 0, 1 }
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

:::::

The next challenge is the implementation of the frontiers.  To obtain
an observably work efficient algorithm, we shall represent frontiers
as arrays.  Let's assume that we have a function called `edge_map`
with the following signature the "edge map" operation. This operation
takes as parameters a graph, an array of atomic flag values, and a
frontier and returns a new frontier.

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ {.cpp}
parray<vtxid_type> edge_map(const adjlist& graph,
                            parray<std::atomic<bool>>& visited,
                            const parray<vtxid_type>& in_frontier);
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Using this function, we can implement the main loop of BFS as shown
below. The algorithm uses the `edge-map` to advance level by level
through the graph. The traversal stops when the frontier is empty.

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ {.cpp}
parray<bool> bfs(const adjlist& graph, vtxid_type source) {
  size_type n = graph.get_nb_vertices();
  parray<std::atomic<bool>> visited(n, std::atomic<bool>(false));
  visited[source].store(true);
  parray<vtxid_type> cur_frontier = { source };
  while (cur_frontier.size() > 0) {
    cur_frontier = edge_map(graph, visited, cur_frontier);
  }
  return visited;
}
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

One minor technical complication relates to the result value: our
algorithm performs extra work to copy out the values from the visited
array. Although it could be avoided, we choose to copy out the values
because it is more convenient for us to program with ordinary
`sparray`'s.  Here is an example describing the behavior of the
`edge_map` function.

::::: {#ex-filter-evens .example}

**Example:** A run of `edge_map`

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ {.cpp}
adjlist graph = // same graph as shown in the previous example
size_type n = graph.get_nb_vertices();
parray<std::atomic<bool>> visited(n, std::atomic<bool>(false));
visited[0].store(true);
visited[1].store(true);
visited[3].store(true);
parray<vtxid_type> in_frontier = { 3 };
parray<vtxid_type> out_frontier = edge_map(graph, visited, in_frontier);
std::cout << out_frontier << std::endl;
parray<vtxid_type> out_frontier2 = edge_map(graph, visited, out_frontier);
std::cout << out_frontier2 << std::endl;
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Output:

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
{ 5, 2 }
{  }
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

:::::

There are several ways to implement `edge_map`.  One way is to
allocate an array that is large enough to hold the next frontier and
then allow the next frontier to be computed in parallel. Since we
don't know the exact size of the next frontier ahead of time, we will
bound it by using the total number of out-going edges originating at
the vertices in the frontier.  To mark unused vertices in this array,
we can use a sentinel value.

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ {.cpp}
const vtxid_type not_a_vertexid = -1l;
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

::::: {#ex-filter-evens .example}

**Example:** Array representation of the next frontier

The following array represents a set of three valid vertex
identifiers, with two positions in the array being empty.

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ {.cpp}
{ 3, not_a_vertexid, 0, 1, not_a_vertexid }
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

:::::

Let us define two helper functions. The first one takes an array of
vertex identifiers and copies out the valid vertex identifiers.

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ {.cpp}
parray<vtxid_type> just_vertexids(const parray<vtxid_type>& vs) {
  return filter(vs.begin(), vs.end(), [&] (vtxid_type v) {
    return v != not_a_vertexid;
  });
}
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

The other function takes a graph and an array of vertex identifiers
and returns the array of the degrees of the vertex identifiers.

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ {.cpp}
parray<size_type> get_out_degrees_of(const adjlist& graph,
                                     const parray<vtxid_type>& vs) {
  parray<size_type> r(vs.size(), [&] (size_type i) {
    return graph.get_out_degree_of(vs[i]);
  });
  return r;
}
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

We can implement edge-map as follows.  We first allocate the array for
the next frontier by upper bounding its size; each element of the
array is initially set to `not_a_vertexid`.  We then, in parallel,
visit each vertex in the frontier and attempt, for each neighbor, to
insert the neighbor into the next frontier by using an atomic
compare-and-swap operations.  If we succeed, then we write the vertex
into the next frontier.  If not, we skip.  Once all neigbors are
visited, we pack the next frontier by removing non-vertices.  This
packed array becomes our next frontier.

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ {.cpp}
parray<vtxid_type> edge_map(const adjlist& graph,
                            parray<std::atomic<bool>>& visited,
                            const parray<vtxid_type>& in_frontier) {
  // temporarily removed.
}
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

The complexity function used by the outer loop in the edge map is
interesting because the complexity function treats the vertices in the
frontier as weighted items. In particular, each vertex is weighted by
its out degree in the graph. The reason that we use such weighting is
because the amount of work involved in processing that vertex is
proportional to its out degree. We cannot treat the out degree as a
constant, unfortunately, because the out degree of any given vertex is
unbounded, in general. As such, it should be clear why we need to
account for the out degrees explicitly in the complexity function of
the outer loop.

::::: {#exercise-bfs-changes .exercise}

**Exercise:** What changes you need to make to BFS to have BFS
annotate each vertex `v` by the length of the shortest path between
`v` and the source vertex?

:::::

### Performance analysis

Our parallel BFS is asymptotically work efficient: the BFS takes work
$O(n + m)$. To establish this bound, we need to assume that the
compare-and-exchange operation takes constant time. After that,
confirming the bound is only a matter of inspecting the code line by
line. On the other hand, the span is more interesting.

::::: {#exercise-bfs-span .exercise}

**Exercise:** What is the span of our parallel BFS?
 
TIP: In order to answer this question, we need to know first about the
graph *diameter*. The diameter of a graph is the length of the
shortest path between the two most distant vertices. It should be
clear that the number of iterations performed by the while loop of the
BFS is at most the same as the diameter.

:::::

::::: {#exercise-bfs-sentinel .exercise}

**Exercise:** By using sentinel values, it might be possible to implement BFS to
eliminate the compaction used by `edge_map.`  Describe and implement
such an algorithm. Does in perform better?  

:::::
				   

Direction Optimization
----------------------

If the graph has reverse edges for each vertex, we can use an
alternative "bottom-up" implementation for `edge_map` instead of the
"top-down" approach described above: instead of scanning the vertices
in the frontier and visiting their neighbors, we can check, for any
unvisited vertex, whether that vertex has a parent that is in the
current frontier. If so, then we add the vertex to the next
frontier. If the frontier is large, this approach can reduce the total
number of edges visited because the vertices in the next frontier will
quickly find a parent.  Furthermore, we don't need use
compare-and-swap operations.  The disadvantage of the bottom-up
approach is that it requires linear work in the number of vertices.
However, if the frontier is already large, the top-down approach
requires lineal work too. Thus if, the frontier is large, e.g., a
constant fraction of the vertices, then the bottom-up approach can be
beneficial.  On current multicore machines, it turns out that if the
frontier is larger than 5% of the total number of vertices, then the
bottom-up approach tends to outperform the top-down approach.

An optimized "hybrid" implementation of PBFS can select between the
top-down and the bottom-up approaches proposed based on the size of
the current frontier.  This is called "direction-optimizing" BFS.

::::: {#exercise-bfs-direction-optimizing .exercise}

**Exercise:** Work out the details of the bottom-up algorithm for
parallel BFS and the hybrid algorithm.  Can you implement and observe
the expected improvements?

:::::
