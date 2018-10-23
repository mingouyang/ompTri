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

-f m (input file format is mmio)

-f t (input file format is tsv)

-g filename (graph file name)

-t num (use num threads; default max)

-v (verbose on; default off)

Examples:

./ompTri -v -f t -g friendster_adj.tsv

./ompTri -v -f m -g friendster_adj.mmio

=======

Notes on performance:

To work with graphs large and small, numbers in upper and lower CSR are stored as 64-bit unsigned integers (uint64_t).  For almost all graphs in the challenge, however, improved performance can be obtained by switching to 32-bit unsigned integers (uint32_t).

=======

Known issue:

The graph 201512020330.v226196185_e480047894.tsv has four self-loops.

./ompTri -v -f t -g 201512020330.v226196185_e480047894.tsv

self-loop: i 175254774, u 44280903

self-loop: i 1050287889, u 109472998

self-loop: i 1051096869, u 109494531

self-loop: i 1051369905, u 109512826

file has 10289801772 bytes, n 226196185, m 240023947

2-core: 211971722 vertices become 0-degree

211971723 vertices of 0-degree are removed

n 14224462, m 28052223, 88 threads

reading file:	33.977310 sec

preprocessing:	46.353436 sec

counting:	0.061484 sec

26 triangles

=======

Another known issue:

The graph friendster_adj.tsv is different from friendster_adj.mmio.  The former has 181,716 triangles, and the latter has 4,173,724,142.
