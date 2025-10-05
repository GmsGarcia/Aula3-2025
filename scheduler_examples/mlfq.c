#include "mlfq.h"

#include <stdio.h>
#include <stdlib.h>

#include "msg.h"
#include <unistd.h>

#include "debug.h"

static const int QUANTA[MLFQ_LEVELS] = { 50, 100, 200 }; // ms per level

static queue_t mlfq_queues[MLFQ_LEVELS]; // priority-based queues

/**
 * Pick next process from highest-priority non-empty queue.
 */
static pcb_t *mlfq_next_task(uint32_t current_time_ms) {
    for (int i = 0; i < MLFQ_LEVELS; i++) {
        pcb_t *t = dequeue_pcb(&mlfq_queues[i]);
        if (t != NULL) {
            t->slice_start_ms = current_time_ms;
            t->priority = i;
            return t;
        }
    }
    return NULL; // all queues empty
}

/**
 * @brief Multilevel Feedback Queue (MLFQ) scheduling algorithm.
 *
 * This function implements the MLFQ scheduling algorithm. If the CPU is not idle it
 * checks if the application is ready and frees the CPU.
 * TODO: description
 *
 * @param current_time_ms The current time in milliseconds.
 * @param rq Pointer to the ready queue containing tasks that are ready to run.
 * @param cpu_task Double pointer to the currently running task. This will be updated
 *                 to point to the next task to run.
 */
void mlfq_scheduler(uint32_t current_time_ms, queue_t *rq, pcb_t **cpu_task) {
    // move all tasks from global ready_queue into MLFQ level 0
    while (rq->head != NULL) {
        pcb_t *t = dequeue_pcb(rq);
        t->priority = 0;
        enqueue_pcb(&mlfq_queues[0], t);
    }

    if (*cpu_task) {
        (*cpu_task)->ellapsed_time_ms += TICKS_MS;

        if ((*cpu_task)->ellapsed_time_ms >= (*cpu_task)->time_ms) {
            msg_t msg = {
                .pid = (*cpu_task)->pid,
                .request = PROCESS_REQUEST_DONE,
                .time_ms = current_time_ms,
                .start_time_ms = (*cpu_task)->start_time_ms
            };
            if (write((*cpu_task)->sockfd, &msg, sizeof(msg_t)) != sizeof(msg_t)) {
                perror("write");
            }

            free(*cpu_task);
            *cpu_task = NULL;
        } else if ((current_time_ms - (*cpu_task)->slice_start_ms) >= QUANTA[(*cpu_task)->priority]) {
            int new_level = (*cpu_task)->priority < (MLFQ_LEVELS - 1) ? (*cpu_task)->priority + 1 : (*cpu_task)->priority;
            (*cpu_task)->slice_start_ms = 0;

            enqueue_pcb(&mlfq_queues[new_level], *cpu_task);

            *cpu_task = NULL;
        }
    }

    if (*cpu_task == NULL) {
        *cpu_task = mlfq_next_task(current_time_ms);

        if (*cpu_task) {
            if ((*cpu_task)->start_time_ms == -1) {
                (*cpu_task)->start_time_ms = current_time_ms - (*cpu_task)->received_time_ms;
            }
        }
    }
}