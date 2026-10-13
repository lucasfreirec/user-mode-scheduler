#include "scheduler.h"

#include <stdlib.h>
#include <string.h>

static Scheduler *active_scheduler = NULL;

/*
 * Releases the resources owned by a task.
 * The task context must no longer be executing when this is called.
 */
static void release_task_stack(Task *task)
{
    if (task->stack != NULL) {
        free(task->stack);
        task->stack = NULL;
    }
}

/*
 * Finds a task by its unique ID.
 */
static Task *find_task_by_id(Scheduler *scheduler, int task_id)
{
    for (int i = 0; i < scheduler->task_count; i++) {
        if (scheduler->tasks[i].id == task_id) {
            return &scheduler->tasks[i];
        }
    }

    return NULL;
}

/*
 * Finds the next READY task using cooperative Round-Robin.
 */
static Task *find_next_task(Scheduler *scheduler)
{
    if (scheduler->task_count == 0) {
        return NULL;
    }

    int start = 0;

    if (scheduler->current_task != NULL) {
        int current_index =
            (int)(scheduler->current_task - scheduler->tasks);

        start = (current_index + 1) % scheduler->task_count;
    }

    for (int i = 0; i < scheduler->task_count; i++) {
        int index = (start + i) % scheduler->task_count;

        if (scheduler->tasks[index].state == TASK_READY) {
            return &scheduler->tasks[index];
        }
    }

    return NULL;
}

/*
 * Entry point for every task.
 *
 * When the user routine returns, the task is marked as FINISHED.
 * Returning from this function transfers control to uc_link,
 * which points to the scheduler context.
 */
static void task_wrapper(void)
{
    Task *task = active_scheduler->current_task;

    task->routine();

    task->state = TASK_FINISHED;
}

/*
 * Initializes the scheduler.
 */
SchedulerResult scheduler_init(Scheduler *scheduler)
{
    if (scheduler == NULL) {
        return SCHEDULER_ERROR_INVALID_ARGUMENT;
    }

    if (active_scheduler != NULL) {
        return SCHEDULER_ERROR_INVALID_STATE;
    }

    memset(scheduler, 0, sizeof(*scheduler));

    for (int i = 0; i < SCHEDULER_MAX_TASKS; i++) {
        scheduler->tasks[i].state = TASK_UNUSED;
    }

    scheduler->task_count = 0;
    scheduler->active_tasks = 0;
    scheduler->next_task_id = 0;
    scheduler->current_task = NULL;
    scheduler->initialized = 1;

    return SCHEDULER_SUCCESS;
}

/*
 * Creates a new task and prepares its execution context.
 */
SchedulerResult scheduler_spawn(
    Scheduler *scheduler,
    TaskRoutine routine,
    int *task_id)
{
    if (scheduler == NULL || routine == NULL || task_id == NULL) {
        return SCHEDULER_ERROR_INVALID_ARGUMENT;
    }

    if (!scheduler->initialized) {
        return SCHEDULER_ERROR_NOT_INITIALIZED;
    }

    if (scheduler->task_count >= SCHEDULER_MAX_TASKS) {
        return SCHEDULER_ERROR_CAPACITY;
    }

    Task *task = &scheduler->tasks[scheduler->task_count];

    task->id = scheduler->next_task_id;
    task->state = TASK_READY;
    task->routine = routine;
    task->stack = NULL;

    /*
     * Capture a context that will be configured for this task.
     */
    if (getcontext(&task->context) == -1) {
        return SCHEDULER_ERROR_SYSTEM;
    }

    /*
     * Allocate an independent stack for the task.
     */
    task->stack = malloc(TASK_STACK_SIZE);

    if (task->stack == NULL) {
        return SCHEDULER_ERROR_OUT_OF_MEMORY;
    }

    task->context.uc_stack.ss_sp = task->stack;
    task->context.uc_stack.ss_size = TASK_STACK_SIZE;
    task->context.uc_stack.ss_flags = 0;

    /*
     * Return to the scheduler when the task routine finishes.
     */
    task->context.uc_link = &scheduler->context;

    /*
     * Configure the task entry point.
     */
    makecontext(&task->context, task_wrapper, 0);

    *task_id = task->id;

    scheduler->task_count++;
    scheduler->active_tasks++;
    scheduler->next_task_id++;

    return SCHEDULER_SUCCESS;
}

/*
 * Cancels a task that has not started executing.
 *
 * Running tasks cannot be cancelled by this operation.
 */
SchedulerResult scheduler_cancel(
    Scheduler *scheduler,
    int task_id)
{
    if (scheduler == NULL) {
        return SCHEDULER_ERROR_INVALID_ARGUMENT;
    }

    if (!scheduler->initialized) {
        return SCHEDULER_ERROR_NOT_INITIALIZED;
    }

    Task *task = find_task_by_id(scheduler, task_id);

    if (task == NULL) {
        return SCHEDULER_ERROR_INVALID_TASK;
    }

    if (task->state != TASK_READY) {
        return SCHEDULER_ERROR_INVALID_STATE;
    }

    task->state = TASK_CANCELLED;

    release_task_stack(task);

    scheduler->active_tasks--;

    return SCHEDULER_SUCCESS;
}

/*
 * Returns the current state of a task.
 */
SchedulerResult scheduler_get_task_state(
    const Scheduler *scheduler,
    int task_id,
    TaskState *state)
{
    if (scheduler == NULL || state == NULL) {
        return SCHEDULER_ERROR_INVALID_ARGUMENT;
    }

    if (!scheduler->initialized) {
        return SCHEDULER_ERROR_NOT_INITIALIZED;
    }

    for (int i = 0; i < scheduler->task_count; i++) {
        if (scheduler->tasks[i].id == task_id) {
            *state = scheduler->tasks[i].state;
            return SCHEDULER_SUCCESS;
        }
    }

    return SCHEDULER_ERROR_INVALID_TASK;
}

/*
 * Runs all READY tasks until no active tasks remain.
 */
SchedulerResult scheduler_run(Scheduler *scheduler)
{
    if (scheduler == NULL) {
        return SCHEDULER_ERROR_INVALID_ARGUMENT;
    }

    if (!scheduler->initialized) {
        return SCHEDULER_ERROR_NOT_INITIALIZED;
    }

    if (active_scheduler != NULL) {
        return SCHEDULER_ERROR_INVALID_STATE;
    }

    active_scheduler = scheduler;

    while (scheduler->active_tasks > 0) {
        Task *next_task = find_next_task(scheduler);

        /*
         * With only READY, RUNNING, FINISHED and CANCELLED states,
         * active tasks should always have a READY task to run
         * whenever the scheduler regains control.
         */
        if (next_task == NULL) {
            active_scheduler = NULL;
            scheduler->current_task = NULL;

            return SCHEDULER_ERROR_SYSTEM;
        }

        scheduler->current_task = next_task;
        next_task->state = TASK_RUNNING;

        /*
         * Save the scheduler context and switch to the task.
         */
        if (swapcontext(
                &scheduler->context,
                &next_task->context) == -1) {

            next_task->state = TASK_READY;
            scheduler->current_task = NULL;
            active_scheduler = NULL;

            return SCHEDULER_ERROR_SYSTEM;
        }

        /*
         * Execution resumes here after task_yield() or when
         * a task finishes.
         */
        if (next_task->state == TASK_FINISHED) {
            release_task_stack(next_task);
            scheduler->active_tasks--;
        }
    }

    scheduler->current_task = NULL;
    active_scheduler = NULL;

    return SCHEDULER_SUCCESS;
}

/*
 * Suspends the currently running task and returns to the scheduler.
 */
SchedulerResult task_yield(void)
{
    if (active_scheduler == NULL ||
        active_scheduler->current_task == NULL) {
        return SCHEDULER_ERROR_INVALID_STATE;
    }

    Scheduler *scheduler = active_scheduler;
    Task *task = scheduler->current_task;

    if (task->state != TASK_RUNNING) {
        return SCHEDULER_ERROR_INVALID_STATE;
    }

    task->state = TASK_READY;

    /*
     * Save the task context and resume the scheduler context.
     */
    if (swapcontext(
            &task->context,
            &scheduler->context) == -1) {

        task->state = TASK_RUNNING;

        return SCHEDULER_ERROR_SYSTEM;
    }

    /*
     * The task resumes here when it is scheduled again.
     */
    return SCHEDULER_SUCCESS;
}

/*
 * Releases all remaining task resources and invalidates the scheduler.
 */
void scheduler_destroy(Scheduler *scheduler)
{
    if (scheduler == NULL || !scheduler->initialized) {
        return;
    }

    /*
     * Do not destroy the scheduler while one of its tasks is running.
     */
    if (active_scheduler == scheduler) {
        return;
    }

    for (int i = 0; i < scheduler->task_count; i++) {
        release_task_stack(&scheduler->tasks[i]);
    }

    scheduler->current_task = NULL;
    scheduler->task_count = 0;
    scheduler->active_tasks = 0;
    scheduler->initialized = 0;
}