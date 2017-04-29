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

#ifndef _GA_SPSCQ
#define _GA_SPSCQ

/*****************************************************************

                LOCK FREE BOUNDED SPSC FIFO QUEUE

  A fast lock free, bounded FIFO queue, for a single producer thread and
  a single consumer thread.

  No (heap) memory allocation is performed in the queue (after creation).

  It is always safe to call ga_spscq_push, ga_spscq_peek and
  ga_spscq_pop, in the sense that the calls will never crash
  and never block (with the exception of ga_spscq_push, if
  the spscq is created with SPSCQ_OVERFLOW_BLOCK).

  If there queue is empty, ga_spscq_read and ga_spscq_peek
  returns NULL. Note that if a NULL value is pushed onto the
  queue, there is no way for the caller of ga_spscq_read
  or ga_spscq_peek to tell if the queue was empty or if it
  got a NULL value from the queue. If NULL values must be
  supported in the queue, the reader thread can call
  ga_spscq_can_read before reading, to check if there is
  anything to read.
  
  If the queue is full, ga_spscq_write will behave according
  to the on_overflow given to ga_spscq_create:

    SPSCQ_OVERFLOW_DISCARD
      The value is silently discarded

    SPSCQ_OVERFLOW_BLOCK
      ga_spscq_write blocks until it can write the value

    SPSCQ_OVERFLOW_GROW
      not implemented

    SPSCQ_OVERFLOW_ERROR
      The value is discarded, and the error callback is called (see below).
      If no error handler is installed, the behavior is undefined
      (currently, an assertion is raised in debug mode, and in
      release mode the value is silently discarded).

    SPSCQ_OVERFLOW_FATAL
      A fatal error is raised (exiting the application)

  An error handler can be installed using ga_spscq_set_error_callback.
  The callback should take four parameters:
    - the queue [ga_spscq*]
    - an error code [ga_error]
    - a error value [void*]  (On GA_ERROR_OVERFLOW, this is the value that couldn't be written)
    - the data parameter passed to ga_spscq_set_error_callback [void*]

 *****************************************************************/

#include <stdlib.h>
#include <ga/util.h>
#include <ga/error.h>

/*
 *  TYPES
 */

typedef struct ga_spscq ga_spscq;

typedef enum ga_spscq_overflow_strategy {
    SPSCQ_OVERFLOW_DISCARD,
    SPSCQ_OVERFLOW_BLOCK,
    SPSCQ_OVERFLOW_GROW,
    SPSCQ_OVERFLOW_ERROR,
    SPSCQ_OVERFLOW_FATAL
} ga_spscq_overflow_strategy;


typedef void (* ga_spscq_callback)(ga_spscq*, ga_error, void*, void*);

/*
 *  FUNCTIONS
 */

ga_spscq* ga_spscq_create(size_t capacity, ga_spscq_overflow_strategy on_overflow);
void ga_spscq_destroy(ga_spscq *queue);

void ga_spscq_set_error_callback(ga_spscq *queue, ga_spscq_callback callback, void *data);

size_t ga_spscq_can_push(ga_spscq *queue);
size_t ga_spscq_can_pop(ga_spscq *queue);
bool ga_spscq_push(ga_spscq *queue, void *value);
void* ga_spscq_pop(ga_spscq *queue);
void* ga_spscq_peek(ga_spscq *queue);
// void ga_spscq_clear(ga_spscq *queue);

void debug_spscq(const ga_spscq *queue);

#endif
