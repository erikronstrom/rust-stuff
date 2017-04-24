#include "ga/queue/mpmcq.h"

#include <stdlib.h>
#include <stdatomic.h>
#include <assert.h>
#include <string.h>
#include <stdio.h>

#include "ga/util.h"
#include "ga/alloc.h"
#include "ga/thread.h"
#include "config.h"

typedef struct cell_t {
    atomic_size_t sequence;
    void* data;
} cell_t;

typedef char cacheline_pad [CACHELINE_SIZE];

struct ga_mpmcq {
  cacheline_pad           pad0;
  cell_t*                 buffer;
  size_t                  buffer_mask;
  cacheline_pad           pad1;
  atomic_size_t           write_pos;
  cacheline_pad           pad2;
  atomic_size_t           read_pos;
  cacheline_pad           pad3;
};

ga_mpmcq* ga_mpmcq_create(size_t capacity)
{
    assert((capacity >= 2) && ((capacity & (capacity - 1)) == 0));
    ga_mpmcq *queue = ga_newc(ga_mpmcq);
    queue->buffer_mask = capacity - 1;
    queue->buffer = ga_calloc(capacity, sizeof(cell_t));
    for (size_t i = 0; i < capacity; i++) {
        atomic_store_explicit(&queue->buffer[i].sequence, i, memory_order_relaxed);
    }
    atomic_store_explicit(&queue->write_pos, 0, memory_order_relaxed);
    atomic_store_explicit(&queue->read_pos, 0, memory_order_relaxed);
    return queue;
}

void ga_mpmcq_destroy(ga_mpmcq *queue)
{
    ga_free(queue->buffer);
    ga_free(queue);
}

bool ga_mpmcq_push(ga_mpmcq *queue, void *value)
{
    cell_t* cell;
    size_t pos = atomic_load_explicit(&queue->write_pos, memory_order_relaxed);
    for (;;)
    {
      cell = &queue->buffer[pos & queue->buffer_mask];
      size_t seq = atomic_load_explicit(&cell->sequence, memory_order_acquire);
      intptr_t dif = (intptr_t)seq - (intptr_t)pos;
      if (dif == 0)
      {
        if (atomic_compare_exchange_weak_explicit(&queue->write_pos, &pos, pos + 1, memory_order_relaxed, memory_order_relaxed))
          break;
      }
      else if (dif < 0)
        return false;
      else
        pos = atomic_load_explicit(&queue->write_pos, memory_order_relaxed);
    }
    cell->data = value;
    atomic_store_explicit(&cell->sequence, pos + 1, memory_order_release);
    return true;
}

void* ga_mpmcq_pop(ga_mpmcq *queue)
{
    cell_t* cell;
    size_t pos = atomic_load_explicit(&queue->read_pos, memory_order_relaxed);
    for (;;)
    {
      cell = &queue->buffer[pos & queue->buffer_mask];
      size_t seq = atomic_load_explicit(&cell->sequence, memory_order_acquire);
      intptr_t dif = (intptr_t)seq - (intptr_t)(pos + 1);
      if (dif == 0)
      {
        if (atomic_compare_exchange_weak_explicit(&queue->read_pos, &pos, pos + 1, memory_order_relaxed, memory_order_relaxed))
          break;
      }
      else if (dif < 0)
        return NULL;
      else
        pos = atomic_load_explicit(&queue->read_pos, memory_order_relaxed);
    }
    void *data = cell->data;
    atomic_store_explicit(&cell->sequence, pos + queue->buffer_mask + 1, memory_order_release);
    return data;
}


// template<typename T>
// class mpmc_bounded_queue
// {
// public:
//   mpmc_bounded_queue(size_t buffer_size)
//     : buffer_(new cell_t [buffer_size])
//     , buffer_mask_(buffer_size - 1)
//   {
//     assert((buffer_size >= 2) &&
//       ((buffer_size & (buffer_size - 1)) == 0));
//     for (size_t i = 0; i != buffer_size; i += 1)
//       buffer_[i].sequence_.store(i, std::memory_order_relaxed);
//     enqueue_pos_.store(0, std::memory_order_relaxed);
//     dequeue_pos_.store(0, std::memory_order_relaxed);
//   }

//   ~mpmc_bounded_queue()
//   {
//     delete [] buffer_;
//   }

//   bool enqueue(T const& data)
//   {
//     cell_t* cell;
//     size_t pos = enqueue_pos_.load(std::memory_order_relaxed);
//     for (;;)
//     {
//       cell = &buffer_[pos & buffer_mask_];
//       size_t seq = 
//         cell->sequence_.load(std::memory_order_acquire);
//       intptr_t dif = (intptr_t)seq - (intptr_t)pos;
//       if (dif == 0)
//       {
//         if (enqueue_pos_.compare_exchange_weak
//             (pos, pos + 1, std::memory_order_relaxed))
//           break;
//       }
//       else if (dif < 0)
//         return false;
//       else
//         pos = enqueue_pos_.load(std::memory_order_relaxed);
//     }
//     cell->data_ = data;
//     cell->sequence_.store(pos + 1, std::memory_order_release);
//     return true;
//   }

//   bool dequeue(T& data)
//   {
//     cell_t* cell;
//     size_t pos = dequeue_pos_.load(std::memory_order_relaxed);
//     for (;;)
//     {
//       cell = &buffer_[pos & buffer_mask_];
//       size_t seq = 
//         cell->sequence_.load(std::memory_order_acquire);
//       intptr_t dif = (intptr_t)seq - (intptr_t)(pos + 1);
//       if (dif == 0)
//       {
//         if (dequeue_pos_.compare_exchange_weak
//             (pos, pos + 1, std::memory_order_relaxed))
//           break;
//       }
//       else if (dif < 0)
//         return false;
//       else
//         pos = dequeue_pos_.load(std::memory_order_relaxed);
//     }
//     data = cell->data_;
//     cell->sequence_.store
//       (pos + buffer_mask_ + 1, std::memory_order_release);
//     return true;
//   }

// private:
//   struct cell_t
//   {
//     std::atomic<size_t>   sequence_;
//     T                     data_;
//   };

//   static size_t const     cacheline_size = 64;
//   typedef char            cacheline_pad_t [cacheline_size];

//   cacheline_pad_t         pad0_;
//   cell_t* const           buffer_;
//   size_t const            buffer_mask_;
//   cacheline_pad_t         pad1_;
//   std::atomic<size_t>     enqueue_pos_;
//   cacheline_pad_t         pad2_;
//   std::atomic<size_t>     dequeue_pos_;
//   cacheline_pad_t         pad3_;

//   mpmc_bounded_queue(mpmc_bounded_queue const&);
//   void operator = (mpmc_bounded_queue const&);
// };

