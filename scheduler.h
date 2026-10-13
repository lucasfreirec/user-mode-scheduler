#ifndef SCHEDULER_H
#define SCHEDULER_H

#include <ucontext.h>

#define SCHEDULER_MAX_TASKS 10
#define TASK_STACK_SIZE (64 * 1024)

typedef void (*TaskRoutine)(void);

typedef enum {
    TASK_UNUSED,
    TASK_READY,
    TASK_RUNNING,
    TASK_FINISHED,
    TASK_CANCELLED
} TaskState;

typedef enum {
    SCHEDULER_SUCCESS,
    SCHEDULER_ERROR_INVALID_ARGUMENT,
    SCHEDULER_ERROR_NOT_INITIALIZED,
    SCHEDULER_ERROR_CAPACITY,
    SCHEDULER_ERROR_OUT_OF_MEMORY,
    SCHEDULER_ERROR_INVALID_TASK,
    SCHEDULER_ERROR_INVALID_STATE,
    SCHEDULER_ERROR_SYSTEM
} SchedulerResult;

typedef struct {
    int id;
    TaskState state;
    ucontext_t context;
    TaskRoutine routine;
    void *stack;
} Task;

typedef struct {
    Task tasks[SCHEDULER_MAX_TASKS];

    int task_count;
    int active_tasks;
    int next_task_id;

    Task *current_task;
    ucontext_t context;

    int initialized;
} Scheduler;

/* Scheduler lifecycle */
SchedulerResult scheduler_init(Scheduler *scheduler);
SchedulerResult scheduler_run(Scheduler *scheduler);
void scheduler_destroy(Scheduler *scheduler);

/* Task lifecycle */
SchedulerResult scheduler_spawn(
    Scheduler *scheduler,
    TaskRoutine routine,
    int *task_id
);

SchedulerResult scheduler_cancel(
    Scheduler *scheduler,
    int task_id
);

SchedulerResult scheduler_get_task_state(
    const Scheduler *scheduler,
    int task_id,
    TaskState *state
);

/* Cooperative scheduling */
SchedulerResult task_yield(void);

#endif