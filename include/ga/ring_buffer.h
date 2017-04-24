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

#ifndef _GA_RING_BUFFER
#define _GA_RING_BUFFER

/*****************************************************************
                    LOCK FREE SPSC RING BUFFER

  A lock free ring buffer for a single producer thread and a
  single consumer thread.

  No (heap) memory allocation is performed in the ring_buffer (after creation).

  It is always safe to call the read and write functions
  (ga_ring_buffer_read, ga_ring_buffer_write etc),
  in the sense that the calls will never crash or block
  - with the exception that calling them with a bytes value
  bigger than the size of the ring buffer results in undefined behavior.

  ga_ring_buffer_write writes the passed number of bytes from the
  passed data buffer into the ring buffer, or until the ring buffer
  is full. It returns the number of bytes actually written.

  ga_ring_buffer_write_atomic works like ga_ring_buffer_write,
  except it's a all-or-nothing: if the complete buffer cannot
  be written, the function returns 0 without writing anything.

  ga_ring_buffer_read reads the passed number of bytes from
  the ring buffer into the passed buffer, or until the ring buffer
  is empty. It returns the number of bytes actually read.

  ga_ring_buffer_read_atomic works like ga_ring_buffer_read,
  except it's a all-or-nothing: if the ring buffer does not
  contain the requested number of bytes, the function returns 0
  and leaves the passed buffer untouched.

  An error handler can be installed using ga_ring_buffer_set_error_callback.
  The callback should take three parameters:
    - the ring buffer [ga_ring_buffer*]
    - an error id [ga_error]
    - the data parameter passed to ga_ring_buffer_set_error_callback [void*]

  The error handler is called when
    - ga_ring_buffer_write cannot write the number of requested bytes (GA_ERROR_OVERFLOW)
    - ga_ring_buffer_read cannot read the number of requested bytes (GA_ERROR_UNDERFLOW)

 *****************************************************************/

#include <stdlib.h>
#include <ga/util.h>
#include <ga/error.h>

/*
 *  TYPES
 */

typedef struct ga_ring_buffer ga_ring_buffer;

typedef void (* ga_ring_buffer_callback)(ga_ring_buffer*, ga_error, void*);

/*
 *  FUNCTIONS
 */

ga_ring_buffer* ga_ring_buffer_create(size_t size);
void ga_ring_buffer_destroy(ga_ring_buffer *ring_buffer);

void ga_ring_buffer_set_error_callback(ga_ring_buffer *ring_buffer, ga_ring_buffer_callback callback, void *data);

size_t ga_ring_buffer_can_read(ga_ring_buffer *ring_buffer);
size_t ga_ring_buffer_can_write(ga_ring_buffer *ring_buffer);
size_t ga_ring_buffer_write(ga_ring_buffer *ring_buffer, size_t bytes, void *data);
size_t ga_ring_buffer_write_atomic(ga_ring_buffer *ring_buffer, size_t bytes, void *data);
size_t ga_ring_buffer_read(ga_ring_buffer *ring_buffer, size_t bytes, void *data);
size_t ga_ring_buffer_read_atomic(ga_ring_buffer *ring_buffer, size_t bytes, void *data);

void debug_ring_buffer(ga_ring_buffer *ring_buffer);

#endif
