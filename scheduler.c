#include "scheduler.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

static Scheduler* active_scheduler = NULL;

void task_wrapper(void) {
    // Calls the function that will be executed
    active_scheduler->current_task->routine();

    printf("Task %d has finished.\n",
           active_scheduler->current_task->id);

    active_scheduler->current_task->state = FINISHED;

    return;
}

void create_task(TaskRoutine routine)
{
    if (active_scheduler->count >= QUEUE_SIZE) {
        printf("Maximum number of tasks reached.\n");
        return;
    }

    Task* new_task =
        &active_scheduler->queue[active_scheduler->count];

    new_task->id = active_scheduler->count;
    new_task->state = READY;
    new_task->routine = routine;

    // Saves the current CPU context to initialize the task context
    getcontext(&new_task->context);

    // VERY IMPORTANT: allocates the stack for each task
    void* stack = malloc(STACK_SIZE);

    new_task->context.uc_stack.ss_sp = stack;
    new_task->context.uc_stack.ss_size = STACK_SIZE;
    new_task->context.uc_link = &active_scheduler->context;

    makecontext(
        &new_task->context,
        task_wrapper,
        0
    );

    active_scheduler->count++;

    printf("Task %d created.\n", new_task->id);
}

// Cooperative Round-Robin
static Task* find_task(void) {
    // Determines where to start searching
    int start =
        (active_scheduler->current_task == NULL)
        ? 0
        : (active_scheduler->current_task->id + 1);

    for (int i = 0; i < active_scheduler->count; ++i) {
        // Circular search
        int index = (start + i) % active_scheduler->count;

        if (active_scheduler->queue[index].state == READY) {
            // Returns the first task that is ready to run
            return &active_scheduler->queue[index];
        }
    }

    // No task is ready; returns NULL
    return NULL;
}

void initialize_scheduler(Scheduler* scheduler) {
    scheduler->count = 0;
    scheduler->current_task = NULL;

    for (int i = 0; i < QUEUE_SIZE; ++i) {
        scheduler->queue[i].state = FINISHED;
    }

    active_scheduler = scheduler;

    printf("Scheduler initialized.\n");
}

void run_scheduler(void) {
    while (1) {
        Task* next_task = find_task();

        if (next_task) {
            active_scheduler->current_task = next_task;
            active_scheduler->current_task->state = RUNNING;

            // Saves the scheduler context and switches to the task context
            swapcontext(
                &active_scheduler->context,
                &active_scheduler->current_task->context
            );
        } else {
            printf("All tasks have finished.\n");
            break;
        }
    }

    active_scheduler->count = 0;
    active_scheduler->current_task = NULL;
}

void yield(void) {
    active_scheduler->current_task->state = READY;

    // Saves the current state and returns control to the scheduler
    swapcontext(
        &active_scheduler->current_task->context,
        &active_scheduler->context
    );
}