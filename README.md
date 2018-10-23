# ompTri
Triangle Counting with A Multi-Core Computer

I worked on this project in response to the GraphChallenge, https://graphchallenge.mit.edu/
The results are described in this paper:
Donato E, Ouyang M, Peguero-Isalguez C.  Triangle counting with a multi-core computer.  Proceedings of IEEE High Performance Extreme Computing Conference (HPEC), 2018, 1-7.
It received Honorable Mention in 2018: https://graphchallenge.mit.edu/champions

To compile:
icc -Wall -O3 -xHost -ipo -qopenmp *.c -o ompTri
