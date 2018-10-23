# ompTri
Triangle Counting with A Multi-Core Computer

I developed this project in response to the GraphChallenge: https://graphchallenge.mit.edu/

The results are described in this paper:

Donato E, Ouyang M, Peguero-Isalguez C.  Triangle counting with a multi-core computer.  Proceedings of IEEE High Performance Extreme Computing Conference (HPEC), 2018, 1-7.

It received Honorable Mention in 2018: https://graphchallenge.mit.edu/champions

=======

To compile:

icc -Wall -O3 -xHost -ipo -qopenmp *.c -o ompTri

or

gcc -Wall -O3 -fopenmp *.c -o ompTri

It was tested on only CentOS 7.5.

=======

Command line options:

-f a (input file format is AdjacencyGraph; default)

-f m (input file format is adjacency mmio)

-f t (input file format is adjacency tsv)

-g filename (graph file name)

-t num (use num threads; default max)

-v (turn on verbose mode; default is off)

Examples:

./ompTri -v -f t -g friendster_adj.tsv

./ompTri -v -f m -g friendster_adj.mmio

=======

Notes on performance:

To work with graphs large and small, numbers in the upper and lower CSR are stored as 64-bit unsigned integers (uint64_t).  For almost all graphs in the challenge, however, improved performance can be obtained by switching to 32-bit unsigned integers (uint32_t).

=======

Known issues:

The graph MAWI 201512020330.v226196185_e480047894.tsv has four self-loops.

The graph friendster_adj.tsv is different from friendster_adj.mmio.  The former has 181,716 triangles, and the latter has 4,173,724,142.
