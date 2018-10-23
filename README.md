# ompTri
Triangle Counting with A Multi-Core Computer

I worked on this project in response to the GraphChallenge: https://graphchallenge.mit.edu/

The results are described in this paper:

Donato E, Ouyang M, Peguero-Isalguez C.  Triangle counting with a multi-core computer.  Proceedings of IEEE High Performance Extreme Computing Conference (HPEC), 2018, 1-7.

It received Honorable Mention in 2018: https://graphchallenge.mit.edu/champions

===

To compile:

icc -Wall -O3 -xHost -ipo -qopenmp *.c -o ompTri

or

gcc -Wall -O3 -fopenmp *.c -o ompTri

Command line options:

-f a (input file format is AdjacencyGraph)

-f m (input file format is mmio)

-f t (input file format is tsv)

-g filename (graph file name)

-t num (use num threads)

-v (verbose)

Examples:

./ompTri -v -f t -g friendster_adj.tsv

./ompTri -v -f m -g friendster_adj.mmio

