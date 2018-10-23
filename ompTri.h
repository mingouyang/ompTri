/* This code is part of this project: Donato E, Ouyang M,
 * Peguero-Isalguez C.  Triangle counting with a multi-core computer.
 * Proceedings of IEEE High Performance Extreme Computing Conference
 * (HPEC), 2018, 1-7.
 *
 * Copyright (c) 2018 Ming Ouyang
 *
 * Permission is hereby granted, free of charge, to any person
 * obtaining a copy of this software and associated documentation
 * files (the "Software"), to deal in the Software without
 * restriction, including without limitation the rights to use, copy,
 * modify, merge, publish, distribute, sublicense, and/or sell copies
 * of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT.  IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
 * HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 * WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 */

//ompTri.c
extern uint64_t verbose;
extern uint64_t numT; //num threads
extern uint64_t n, m; //number of vertices, number of edges
extern uint64_t *degree, **neighbor; //degree[n], *neighbor[n]

//adj.c
extern void readAdjGraph(char *filename);

//brkTie.c
extern uint64_t *idx;   //vertex numbers from input file
extern uint64_t *newID; //IDs after tiebreaking sorting
extern void sortBrkTie(void);

//dfs.c
extern void dfs2core(void);

//luCSR.c
extern uint64_t *lower, *upper, *lowerOffset, *upperOffset; //luCSR
extern uint64_t *rowNum;
extern void luCSR(void);

//mmio.c
extern void readMMIO(char *filename);

//simple.c
extern void toSimpleGraph(void);

//tsv.c
extern void readTSV(char *filename);

//utils.c
extern uint64_t str2u64(const char s[]);
