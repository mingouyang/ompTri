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
#include <time.h>
#include <sys/time.h>
#include <omp.h>

#include "ompTri.h"

#define MILLION 1000000L

uint64_t verbose, numT, n, m, *degree, **neighbor;

/* The Forward algorithm
 *
 * T. Schank, "Algorithmic Aspects of Triangle-Based Network
 * Analysis," Ph.D. dissertation, Universitat Karlsruhe, 2007.
 *
 * modified to use luCSR that stores the upper and lower triangles
 * separately in two CSRs
 */
static uint64_t forward(void) {
  uint64_t i, j, k, v, w, count = 0;

#pragma omp parallel for private(j,k,v,w) reduction(+:count) schedule(guided)
  for (i = upperOffset[1]; i < m; i++) {
    v = rowNum[i];
    w = upper[i];
    j = lowerOffset[v];
    k = lowerOffset[w];
    while (j < lowerOffset[v + 1] && lower[k] < v && k < lowerOffset[w + 1]) {
      if (lower[j] < lower[k]) {
	j++;
	continue;
      }
      if (lower[j] > lower[k]) {
	k++;
	continue;
      }
      count++, j++, k++;
    }
  }

  return count;
}

int main(int argc, char* argv[]) {
  struct timeval start, stop;
  char *filename, format;
  float t1, t2, t3;
  uint64_t count;
  int c;

  verbose = 0;
  format = 'a';
  filename = NULL;
  numT = omp_get_max_threads();
  while ((c = getopt(argc, argv, "f:g:t:v")) != -1) {
    switch (c) {
    case 'f': //file format, one char
      format = optarg[0];
      /* a: AdjacencyGraph
       * m: mmio
       * t: tsv
       */
      break;
    case 'g': //file name
      filename = (char*)malloc(sizeof(char) * (strlen(optarg) + 1));
      strcpy(filename, optarg);
      break;
    case 't': //number of threads
      sscanf(optarg, "%lu", &numT);
      break;
    case 'v': //verbose
      verbose = 1;
      break;
    default: break;
    }
  }
  if (format != 'a' && format != 'm' && format != 't') {
    fprintf(stderr, "file format:\n");
    fprintf(stderr, "\ta for AdjacencyGraph\n\tm for mmio\n\tt for tsv\n");
    return 0;
  }
  if (!filename) {
    fprintf(stderr, "filename?\n");
    return 0;
  }
  if (numT < 2 || numT > omp_get_max_threads())
    numT = omp_get_max_threads();
  omp_set_num_threads(numT);

  gettimeofday(&start, NULL);
  switch (format) {
  case 'a':
    readAdjGraph(filename);
    break;
  case 'm':
    readMMIO(filename);
    break;
  case 't':
    readTSV(filename);
    break;
  default:
    break;
  }
  gettimeofday(&stop, NULL);
  t1 = (stop.tv_sec - start.tv_sec) +
    (stop.tv_usec - start.tv_usec) / (float)MILLION; //reading file

  gettimeofday(&start, NULL);
  toSimpleGraph(); //useless
  dfs2core();
  sortBrkTie();
  luCSR();
  gettimeofday(&stop, NULL);
  t2 = (stop.tv_sec - start.tv_sec) +
    (stop.tv_usec - start.tv_usec) / (float)MILLION; //preprocessing

  gettimeofday(&start, NULL);
  count = forward();
  gettimeofday(&stop, NULL);
  t3 = (stop.tv_sec - start.tv_sec) +
    (stop.tv_usec - start.tv_usec) / (float)MILLION; //counting

  if (verbose) {
    printf("n %lu, m %lu, %lu threads\n", n, m, numT);
    printf("reading file:\t%.6f sec\n", t1);
    printf("preprocessing:\t%.6f sec\n", t2);
    printf("counting:\t%.6f sec\n", t3);
    printf("%lu triangles\n", count);
  }
  else
    printf("%lu triangles, %.6f sec\n", count, t3);

  return 0;
}
