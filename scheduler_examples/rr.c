#include "rr.h"

#include <stdio.h>
#include <stdlib.h>

#include "msg.h"
#include <unistd.h>

/**
 * @brief Round Robin (RR) scheduling algorithm.
 *
 * This function implements the RR scheduling algorithm. If the CPU is not idle it
 * checks if the application is ready and frees the CPU.
 * TODO: description
 *
 * @param current_time_ms The current time in milliseconds.
 * @param rq Pointer to the ready queue containing tasks that are ready to run.
 * @param cpu_task Double pointer to the currently running task. This will be updated
 *                 to point to the next task to run.
 */
void rr_scheduler(uint32_t current_time_ms, queue_t *rq, pcb_t **cpu_task) {
    if (*cpu_task) {
        (*cpu_task)->ellapsed_time_ms += TICKS_MS;      // Add to the running time of the application/task

        if ((*cpu_task)->ellapsed_time_ms >= (*cpu_task)->time_ms) {
            // Task finished
            // Send msg to application
            msg_t msg = {
                .pid = (*cpu_task)->pid,
                .request = PROCESS_REQUEST_DONE,
                .time_ms = current_time_ms,
                .start_time_ms = (*cpu_task)->start_time_ms
            };
            if (write((*cpu_task)->sockfd, &msg, sizeof(msg_t)) != sizeof(msg_t)) {
                perror("write");
            }
            // Application finished and can be removed
            free((*cpu_task));
            (*cpu_task) = NULL;
        } else if ((current_time_ms - (*cpu_task)->slice_start_ms) >= TIME_QUANTUM_MS) {
            // time slice is up!
            // run next pcb
            (*cpu_task)->slice_start_ms = 0;
            enqueue_pcb(rq, *cpu_task);
            (*cpu_task) = NULL;
        }
    }

    if (*cpu_task == NULL) {            // If CPU is idle
        *cpu_task = dequeue_pcb(rq);   // Get next task from ready queue (dequeue from head)

        if (*cpu_task) {
            (*cpu_task)->slice_start_ms = current_time_ms;

            if ((*cpu_task)->start_time_ms == -1) {
                (*cpu_task)->start_time_ms = current_time_ms - (*cpu_task)->received_time_ms;
            }
        }

    }
}