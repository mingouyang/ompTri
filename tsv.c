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

/* read a tsv file
 * store the graph in degree[n] and neighbor[n][]
 *
 * tsv format: one edge per line, each line is 3 numbers separated by 2 tabs
 * "u\tv\tedgeWeight\n"
 * vertex numbers, u and v, start at 1
 */
void readTSV(char *filename) {
  uint64_t i, j, *off, numItem, *myNumItem, chunk, start, end;
  uint64_t *rawNum, u, v;
  struct stat buf;
  char *buffer;
  int status;
  FILE *fp;

  status = stat(filename, &buf);
  if (status) {
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
  fread((void*) buffer, 1, buf.st_size, fp);
  fclose(fp);

  //count how many numbers are in the file
#pragma omp parallel for private(j,start,end)
  for (i = 0; i < numT; i++) {
    start = i * chunk;
    end = (i == numT - 1) ? buf.st_size : start + chunk;
    for (j = start; j < end; j++)
      if (buffer[j] == '\t' || buffer[j] == '\n')
	myNumItem[i + 1]++; //note (i + 1), shift by one
  }

  for (i = 0; i < numT; i++) //prefix sum
    myNumItem[i + 1] += myNumItem[i];
  numItem = myNumItem[numT]; //number of numbers in the file
  off =    (uint64_t*) malloc(sizeof(uint64_t) * (numItem + 1));
  rawNum = (uint64_t*) malloc(sizeof(uint64_t) * numItem);

  off[0] = 0;
  off += 1;

  //locate the beginning of each number in the file
#pragma omp parallel for private(j,start,end)
  for (i = 0; i < numT; i++) {
    start = i * chunk;
    end = (i == numT - 1) ? buf.st_size : start + chunk;
    for (j = start; j < end; j++)
      if (buffer[j] == '\t' || buffer[j] == '\n')
	off[ myNumItem[i]++ ] = j + 1;
  }

  off -= 1;
  n = 0; //for max reduction

#pragma omp parallel for reduction(max:n)
  for (i = 0; i < numItem; i++) {
    rawNum[i] = str2u64( &buffer[ off[i] ]);
    n = (n < rawNum[i]) ? rawNum[i] : n;
  }

  free(off);
  free(myNumItem);
  free(buffer);
  degree   = (uint64_t*) malloc(sizeof(uint64_t) * n);
  neighbor = (uint64_t**)malloc(sizeof(uint64_t*) * n);

  for (i = 0; i < n; i++)
    degree[i] = 0;
  //vertex numbers in a tsv file start at 1
  for (i = 0; i < numItem; i += 3)
    degree[rawNum[i] - 1]++; //shift to 0-based indexing
  for (i = 0; i < n; i++) {
    if (degree[i])
      neighbor[i] = (uint64_t*) malloc(sizeof(uint64_t) * degree[i]);
    else
      neighbor[i] = NULL;
    degree[i] = 0;
  }

  for (i = 0; i < numItem; i += 3) {
    u = rawNum[i] - 1;
    v = rawNum[i + 1] - 1;
    if (u == v) {
      fprintf(stderr, "self-loop: i %lu, u %lu\n", i, u);
      continue;
    }
    neighbor[u] [degree[u]++] = v;
  }

  free(rawNum);

  if (verbose)
    printf("n %lu, m %lu\n", n, numItem / 3 >> 1); //m is not set yet
}

//memory not freed: degree, neighbor, neighbor[*]
