#ifndef _GA_QUEUE
#define _GA_QUEUE

#include "ga/queue/spscq.h"
#include "ga/queue/mpmcq.h"
#include "ga/queue/prioq.h"

#define ga_queue_push(queue, value) _Generic((queue), \
              ga_spscq*: ga_spscq_push, \
              ga_mpmcq*: ga_mpmcq_push, \
              ga_prioq*: ga_prioq_push  \
)(queue, value)

#define ga_queue_pop(queue) _Generic((queue), \
              ga_spscq*: ga_spscq_pop, \
              ga_mpmcq*: ga_mpmcq_pop, \
              ga_prioq*: ga_prioq_pop  \
)(queue)

#define ga_queue_peek(queue) _Generic((queue), \
              ga_spscq*: ga_spscq_peek, \
              ga_prioq*: ga_prioq_peek  \
)(queue)


#endif
