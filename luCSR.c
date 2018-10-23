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

uint64_t *lower, *upper, *lowerOffset, *upperOffset, *rowNum;

//prepare the upper and lower CSR
void luCSR(void) {
  uint64_t i, j, a, b, r, *midPt;

  //use 64B alignment to reduce cache coherence traffic
  posix_memalign((void **)&lowerOffset, 64, sizeof(uint64_t) * (n + 1));
  posix_memalign((void **)&upperOffset, 64, sizeof(uint64_t) * (n + 1));
  posix_memalign((void **)&midPt,       64, sizeof(uint64_t) * n);

  lowerOffset[0] = upperOffset[0] = 0;
  for (i = 0; i < n; i++) {
    for (j=0;
	 j < degree[ idx[i] ] &&
	 newID[ neighbor[ idx[i] ] [j] ] < i;
	 j++)
      ;
    midPt[i] = j;
    lowerOffset[i + 1] = lowerOffset[i] + j;
    upperOffset[i + 1] = upperOffset[i] + degree[ idx[i] ] - j;
  }
  m = lowerOffset[n]; //set m for the first time

  posix_memalign((void **)&lower,  64, sizeof(uint64_t) * m);
  posix_memalign((void **)&upper,  64, sizeof(uint64_t) * m);
  posix_memalign((void **)&rowNum, 64, sizeof(uint64_t) * m);

  a = b = 0;
  for (i = 0; i < n; i++) {
    for (j = 0; j < midPt[i]; j++)
      lower[a++] = newID[ neighbor[ idx[i] ] [j] ];
    for (j = midPt[i]; j < degree[ idx[i] ]; j++)
      upper[b++] = newID[ neighbor[ idx[i] ] [j] ];
  }

  r = 0;
  for (i = 0; i < m; i++) {
    while (i >= upperOffset[r + 1])
      r++;
    rowNum[i] = r;
  }

  free(midPt);
}

//memory not freed: lower, upper, lowerOffset, upperOffset, rowNum
//they are used by the forward algorithm
