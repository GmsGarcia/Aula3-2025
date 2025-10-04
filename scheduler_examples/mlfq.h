#ifndef MLFQ_H
#define MLFQ_H

#include "queue.h"

#define MLFQ_LEVELS 3

static pcb_t *mlfq_next_task(uint32_t current_time_ms);

void mlfq_scheduler(uint32_t current_time_ms, queue_t *rq, pcb_t **cpu_task);

#endif //MLFQ_H