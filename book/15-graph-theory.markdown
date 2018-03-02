Graph theory
============

This chapter is a brief overview of some of the graph terminology used
in this book. For the definitions, we assume undirected graphs but the
definitions generalize straightforwardly for the case of directed
graphs.

Walks, trails, paths
--------------------

A ***walk*** in a graph is a sequence of alternating vertex-edge
pairs, $v_0, e_1, v_1, \ldots, v_{n-1}, e_n, v_n$,such that $e_i =
(v_{i-1}, v_i) \in E$. Note that vertices and edges can be repeated. A
walk corresponds to the intuitive notion of "taking a walk in the
graph."

In a simple graph, where there are no parallel or multiple edges, we
can specify a walk as a sequence of vertices (which imply) the edges.

A ***trail*** in a graph is a walk where all edges are distinct, that
is no edge is traversed more than once. Note that there can be
repeated vertices in a trail. A trail corresponds to the intuitive
notion of the "trail" of a walk.

A ***path*** is a graph is a walk where all edges and vertices are
distinct.

We say that a walk or a trail is closed if the initial and terminal
vertices are the same.

Euler tours
-----------

Given an undirected graph $G=(V,E)$, an ***Euler trail*** is a trail
that uses each edge exactly once. An Euler tour is a trail that is
closed. In other words, it starts and terminates at the same
vertex. We say that a graph is ***Eulerian*** if it has an Euler tour.

It can be proven that a connected graph $G=(V,E)$ is Eulerian if and
only if every vertex $v \in V$ has even degree.

Trees
-----

A ***tree*** is an undirected graph in which any two of vertices are
connected by exactly one path.

A ***forest*** is a set of disjoint trees.

A ***rooted tree*** is a tree with a designated root.

A rooted tree may be directed by pointing all edges towards the root or away from the root. Both are acceptable.

For a rooted tree, we can define the a parent-child relationship
between nodes. If a node $u$ is the first node on the path that
connects another node $v$ to the root, then $u$ is the parent of $v$
and $v$ is the child of $u$.

An ***ordered tree*** is a tree where the children of each node are
totally ordered.

Binary trees
------------

A ***binary tree*** is a rooted, directed, ordered tree where each
node has has at most two children, called the left and the right
child, corresponding to the first and the second respectively.

For a binary tree, we can define couple of different kinds of
traversals. An ***in-order traversal*** traverses the binary tree by
performing an in-order traversal of the left subtree, visiting the
root, and then performing an in-order traversal of the right
subtree. A ***post-order traversal*** traverses the binary tree by
performing an post-order traversal of the right subtree, visiting the
root, and then performing an post-order traversal of the right
subtree. An ***pre-order traversal*** traverses the binary tree by
visiting the root, performing an pre-order traversal of the left
subtree, and then performing an pre-order traversal of the right
subtree.

A ***full binary tree*** is a binary tree, where each non-leaf node
has degree exatly two.

A ***complete binary tree*** is a full binary tree, where all the
levels except possibly the last are completely full.

A ***perfect binary tree*** is a full binary tree where all the leaves
are at the same level.
