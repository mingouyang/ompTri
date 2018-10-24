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
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <omp.h>

#include "ompTri.h"

#define SIZE 4096

/* read an mmio file
 * store the graph in degree[n] and neighbor[n][]
 *
 * mmio format:
 * two lines of comments
 * third line: n n m
 * from 4th to (m+3)th lines: u v weight
 * vertex numbers, u and v, start at 1
 */
void readMMIO(char *filename) {
  uint64_t i, u, v, tmp, *U, *V;
  char buffer[SIZE];
  FILE *fp;

  fp = fopen(filename, "r");
  if (!fp) {
    printf("no such file: %s\n", filename);
    exit(0);
  }
  fgets(buffer, SIZE, fp);
  fgets(buffer, SIZE, fp);
  fgets(buffer, SIZE, fp);
  sscanf(buffer, "%lu %lu %lu", &n, &tmp, &m);
  if (verbose)
    printf("n %lu, m %lu\n", n, m);
  
  neighbor = (uint64_t**)malloc(sizeof(uint64_t*) * n);
  degree   = (uint64_t *)malloc(sizeof(uint64_t) * n);
  U        = (uint64_t *)malloc(sizeof(uint64_t) * m);
  V        = (uint64_t *)malloc(sizeof(uint64_t) * m);

  for (i = 0; i < n; i++)
    degree[i] = 0;
  for (i = 0; i < m; i++) {
    if (fgets(buffer, SIZE, fp) == NULL) {
      fprintf(stderr, "%s is too short\n", filename);
      exit(0);
    }
    sscanf(buffer, "%lu %lu", &u, &v); //ignore the third number, weight
    //vertex numbers in an MMIO file start at 1
    u--, v--; //shift to 0-based indexing
    U[i] = u, V[i] = v;
    degree[u]++, degree[v]++; //undirected graphs
  }
  fclose(fp);

  for (i = 0; i < n; i++) {
    if (degree[i])
      neighbor[i] = (uint64_t*)malloc(sizeof(uint64_t) * degree[i]);
    else
      neighbor[i] = NULL;
    degree[i] = 0;
  }

  for (i = 0; i < m; i++) {
    u = U[i], v = V[i];
    neighbor[u] [ degree[u]++ ] = v;
    neighbor[v] [ degree[v]++ ] = u;
  }

  free(V);
  free(U);
}

//memory not freed: degree, neighbor, neighbor[*]
