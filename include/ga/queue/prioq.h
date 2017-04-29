/*
  
  Copyright (c) 2017 Erik Ronström
  
  Permission is hereby granted, free of charge, to any person obtaining a copy
  of this software and associated documentation files (the "Software"), to deal
  in the Software without restriction, including without limitation the rights
  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
  copies of the Software, and to permit persons to whom the Software is
  furnished to do so, subject to the following conditions:
  
  The above copyright notice and this permission notice shall be included in
  all copies or substantial portions of the Software.
  
  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
  THE SOFTWARE.

*/

#ifndef _GA_PRIOQ
#define _GA_PRIOQ

/*****************************************************************

                NON-THREAD-SAFE PRIORITY QUEUE

  A mutable skew heap (see http://en.wikipedia.org/wiki/Skew_heap)

  NOTE: ga_prioq is NOT thread safe!

  Nodes are reused, to minimize allocation. They can also be
  preallocated, using ga_prioq_preallocate.

  A compare function has to be provided, so that the queue knows
  how to sort its members. The compare function can look something
  like

  int cmp(void *a, void *b) {
    return ((mystruct*)a)->id - ((mystruct*)b)->id;
  }

 *****************************************************************/

#include <stdlib.h>
#include <ga/util.h>
#include <ga/error.h>

/*
 *  TYPES
 */

typedef struct ga_prioq ga_prioq;

typedef int (* ga_prioq_cmpfn)(void*, void*);

/*
 *  FUNCTIONS
 */

ga_prioq* ga_prioq_create(ga_prioq_cmpfn cmpfn);
void ga_prioq_destroy(ga_prioq *prioq);
void ga_prioq_preallocate(ga_prioq *queue, unsigned int count);

void ga_prioq_push(ga_prioq *queue, void *value);
void* ga_prioq_pop(ga_prioq *queue);
void* ga_prioq_peek(const ga_prioq *queue);
// void ga_prioq_clear(ga_prioq *queue);

void debug_prioq(ga_prioq *squeue);

#endif
