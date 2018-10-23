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

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#include "ompTri.h"

void dfs2core(void) {
  uint64_t root, *visited, *uStack, *dStack, stackTop, u, v, d, parent;
  uint64_t i, j, k, count = 0, *newDeg;
  //count is the number of vertices that are removed

  newDeg  = (uint64_t*) malloc(sizeof(uint64_t) * n);
  visited = (uint64_t*) malloc(sizeof(uint64_t) * n);
  uStack  = (uint64_t*) malloc(sizeof(uint64_t) * n);
  dStack  = (uint64_t*) malloc(sizeof(uint64_t) * n);

  root = 0;
  //use the vertex with the highest degree as the root of DFS
  //omp max reduction?
  for (i = 0; i < n; i++) {
    root = (degree[i] > degree[root]) ? i : root;
    visited[i] = 0;        //all vertices will be visited
    newDeg[i] = degree[i]; //to be decremented during shrinkage to 2-core
  }
  stackTop = 0;            //push root
  uStack[stackTop] = root; //uStack: the vertex u to be visited
  dStack[stackTop++] = 0;  //dStack: how far along the neighbors of u

  while (stackTop) {
    u = uStack[stackTop - 1]; //not popping u yet
    d = dStack[stackTop - 1];
    if (d == 0) //visit u for the first time
      visited[u] = 1;

    for (i = d; i < degree[u]; i++) {
      v = neighbor[u] [i];
      if (visited[v])
	continue;
      dStack[stackTop - 1] = i + 1; //update how far along the neighbors of u
      uStack[stackTop] = v;         //push v
      dStack[stackTop++] = 0;
      break;
    }

    if (uStack[stackTop - 1] != u) //v has been pushed into stack
      continue;                    //to process v

    stackTop--; //pop u now -- we have visited all neighbors of u

    if (newDeg[u] == 1) { //remove u
      count++;
      newDeg[u] = 0;
      parent = uStack[stackTop - 1];
      newDeg[parent]--;
    }
  }

  //purge 0-degree vertices from lists of neighbors
  for (i = 0; i < n; i++) {
    if (newDeg[i] == degree[i])
      continue;
    if (newDeg[i] == 0) {
      degree[i] = 0;
      continue;
    }
    for (j = k = 0; j < degree[i]; j++)
      if (newDeg[ neighbor[i] [j]])
	neighbor[i] [k++] = neighbor[i] [j];
    degree[i] = k; //k should be == newDeg[i]
  }

  free(dStack);
  free(uStack);
  free(visited);
  free(newDeg);

  if (verbose)
    printf("2-core: %lu vertices become 0-degree\n", count);
}

//all allocated memory are freed
