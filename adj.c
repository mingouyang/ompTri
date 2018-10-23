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

/* read an AdjacencyGraph file
 * store the graph in degree[n] and neighbor[n][]
 *
 * File format: See Ligra: A Lightweight Graph Processing Framework
 * for Shared Memory", presented at Principles and Practice of
 * Parallel Programming, 2013.
 */
void readAdjGraph(char *filename) {
  uint64_t i, j, *off, chunk, start, end, numItem, *myNumItem;
  uint64_t *row, *rowOffset;
  struct stat buf;
  char *buffer;
  int status;
  FILE *fp;

  status = stat(filename, &buf);
  if (status){
    printf("no such file: %s\n", filename);
    exit(0);
  }
  if (verbose)
    printf("file has %lu bytes, ", buf.st_size);

  buffer = (char*) malloc(buf.st_size);
  myNumItem = (uint64_t*) malloc(sizeof(uint64_t) * (numT + 1));
  for (i = 0; i <= numT; i++) //prefix sum later, need one extra element
    myNumItem[i] = 0;
  chunk = buf.st_size / numT;

  //grab the whole file
  fp = fopen(filename, "rb");
  fread((void*)buffer, 1, buf.st_size, fp);
  fclose(fp);

  //count how many numbers are in the file
#pragma omp parallel for private(j,start,end)
  for (i = 0; i < numT; i++) {
    start = i * chunk;
    end = (i == numT - 1) ? buf.st_size : start + chunk;
    for (j = start; j < end; j++)
      if (buffer[j] == '\n')
	myNumItem[i + 1]++; //note (i + 1), shift by one
  }

  for (i = 0; i < numT; i++) //prefix sum
    myNumItem[i + 1] += myNumItem[i];
  numItem = myNumItem[numT]; //number of numbers in the file
  off = (uint64_t*) malloc(sizeof(uint64_t) * numItem);

  //locate the beginning of each number in the file
#pragma omp parallel for private(j,start,end)
  for (i = 0; i < numT; i++) {
    start = i * chunk;
    end = (i == numT - 1) ? buf.st_size : start + chunk;
    for (j = start; j < end; j++)
      if (buffer[j] == '\n')
	off[ myNumItem[i]++ ] = j + 1;
  }

  buffer[off[0] - 1] = 0;
  if (strcmp(buffer, "AdjacencyGraph") != 0){
    fprintf(stderr, "file format is not AdjacencyGraph\n");
    exit(0);
  }

  n = str2u64(buffer + off[0]);
  m = str2u64(buffer + off[1]);
  rowOffset = (uint64_t*) malloc(sizeof(uint64_t) * (n + 1));
  row       = (uint64_t*) malloc(sizeof(uint64_t) * m);
  if (verbose)
    printf("n %lu, m %lu\n", n, m >> 1);

  //vertex numbers in an AdjacencyGraph file start at 0
#pragma omp parallel for
  for (i = 0; i < n; i++)
    rowOffset[i] = str2u64(buffer + off[i + 2]);

  rowOffset[n] = m;

#pragma omp parallel for
  for (i = 0; i < m; i++)
    row[i] = str2u64(buffer + off[i + n + 2]);

  free(off);
  free(myNumItem);
  free(buffer);
  degree   = (uint64_t*)malloc(sizeof(uint64_t) * n);
  neighbor = (uint64_t**)malloc(sizeof(uint64_t*) * n);

  for (i = 0; i < n; i++) {
    degree[i] = rowOffset[i + 1] - rowOffset[i];
    if (degree[i]) {
      neighbor[i] = (uint64_t*)malloc(sizeof(uint64_t) * degree[i]);
      for (j = 0; j < degree[i]; j++)
	neighbor[i] [j] = row[ rowOffset[i] + j ];
    }
  }

  free(row);
  free(rowOffset);
}

//memory not freed: degree, neighbor, neighbor[*]
