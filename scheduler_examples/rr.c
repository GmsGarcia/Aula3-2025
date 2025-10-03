#include "sjf.h"

#include <stdio.h>
#include <stdlib.h>

#include "msg.h"
#include <unistd.h>

/**
 * @brief Shortest-Job-First (SJF) scheduling algorithm.
 *
 * This function implements the SJF scheduling algorithm. If the CPU is not idle it
 * checks if the application is ready and frees the CPU.
 * TODO: If the CPU is idle, it selects the next task to run based on the order they were added
 * to the ready queue. The task that has been in the queue the longest is selected to run next.
 *
 * @param current_time_ms The current time in milliseconds.
 * @param rq Pointer to the ready queue containing tasks that are ready to run.
 * @param cpu_task Double pointer to the currently running task. This will be updated
 *                 to point to the next task to run.
 */
// TODO:
void sjf_scheduler(uint32_t current_time_ms, queue_t *rq, pcb_t **cpu_task) {
    if (*cpu_task) {
        (*cpu_task)->ellapsed_time_ms += TICKS_MS;      // Add to the running time of the application/task

        if ((*cpu_task)->ellapsed_time_ms >= (*cpu_task)->time_ms) {
            // Task finished
            // Send msg to application
            msg_t msg = {
                .pid = (*cpu_task)->pid,
                .request = PROCESS_REQUEST_DONE,
                .time_ms = current_time_ms
            };
            if (write((*cpu_task)->sockfd, &msg, sizeof(msg_t)) != sizeof(msg_t)) {
                perror("write");
            }
            // Application finished and can be removed
            free((*cpu_task));
            (*cpu_task) = NULL;
        }
    }

    if (*cpu_task == NULL) {            // If CPU is idle
        queue_elem_t *sj = NULL;
        queue_elem_t *curr = rq->head;

        while (curr != NULL) {
            // first iteration...
            if (sj == NULL) {
                sj = curr;
            }

            uint32_t sj_rem = sj->pcb->time_ms - sj->pcb->ellapsed_time_ms;
            uint32_t curr_rem = curr->pcb->time_ms - curr->pcb->ellapsed_time_ms;

            if (sj_rem > curr_rem) {
                sj = curr;
            }

           curr = curr->next;
        }

        if (sj) {
            remove_queue_elem(rq, sj);
            *cpu_task = sj->pcb;
            free(sj);
        }
        /*
        if (sj) {
            queue_elem_t *job = remove_queue_elem(rq, sj);
            if (job) {
                *cpu_task = job->pcb;
                free(job);
            }
        }
        */
    }
}