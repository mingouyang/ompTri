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
#include <omp.h>

#include "ompTri.h"

uint64_t *idx, *newID;

static uint64_t *hub;
static uint64_t oneBit[64];
//oneBit[0] 0001
//oneBit[1] 0010
//oneBit[2] 0100
//oneBit[3] 1000

//descending
static int cmpDeg(const void *a, const void *b) {
  return degree[*(uint64_t*) b] - degree[*(uint64_t*) a];
}

//ascending
static int cmpNewID(const void *a, const void *b) {
  return newID[*(uint64_t*) a] - newID[*(uint64_t*) b];
}

//descending degree, break tie by adjacency to hubs
static int degBrkTie(const void *a, const void *b) {
  uint64_t u, v;

  u = *(uint64_t*) a;
  v = *(uint64_t*) b;
  if (degree[u] > degree[v])
    return -1;
  if (degree[u] < degree[v])
    return 1;
  if (hub[u] > hub[v])
    return -1;
  if (hub[u] < hub[v])
    return 1;
  return 0;
}

void sortBrkTie(void) {
  uint64_t i, j, u, count;

  oneBit[0] = 1;
  for (i = 1; i < 64; i++)
    oneBit[i] = oneBit[i - 1] << 1;

  idx   = (uint64_t*) malloc(sizeof(uint64_t) * n);
  newID = (uint64_t*) malloc(sizeof(uint64_t) * n);
  hub   = (uint64_t*) malloc(sizeof(uint64_t) * n);
  for (i = 0; i < n; i++) {
    idx[i] = i;
    hub[i] = 0;
  }

  //sort all vertices by their degrees to find the top 64 hubs
  //improve this by partitioning
  qsort((void *)idx, n, sizeof(uint64_t), cmpDeg);

  //adjacency to the top 64 hub vertices
  for (i = 0; i < 64; i++)
    for (j = 0; j < degree[ idx[i] ]; j++) {
      u = neighbor[ idx[i] ] [j];
      hub[u] = hub[u] | oneBit[64 - i - 1];
    }

  //sort by degrees, tiebreaking with adjacency to hubs
  qsort((void *)idx, n, sizeof(uint64_t), degBrkTie);
  for (i = 0; i < n; i++)
    newID[ idx[i] ] = i;

  //sort neighbors by their new indices
#pragma omp parallel for schedule(guided)
  for (i = 0; i < n; i++)
    if (degree[i] > 1)
      qsort((void *) neighbor[i], degree[i], sizeof(uint64_t), cmpNewID);

  count = 0;
  //drop zero-degree vertices
  while (degree[ idx[n - 1] ] == 0) {
    if (neighbor[ idx[n - 1] ]) {
      free(neighbor[ idx[n - 1] ]);
      neighbor[ idx[n - 1] ] = NULL;
    }
    count++, n--;
  }

  free(hub);

  if (verbose)
    printf("%lu vertices of 0-degree are removed\n", count);
}

//memory not freed: idx, newID -- they are needed by luCSR.c
