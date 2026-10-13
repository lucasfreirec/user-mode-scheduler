#ifndef SCHEDULER_H
#define SCHEDULER_H

#include <ucontext.h>

#define QUEUE_SIZE 10
#define STACK_SIZE 65546 // 64 KB

typedef void (*TaskRoutine)(void);

typedef enum {
    READY,
    RUNNING,
    FINISHED
} TaskState;

typedef struct {
    int id;
    TaskState state;
    ucontext_t context;
    TaskRoutine routine;
} Task;

typedef struct {
    int count;
    Task queue[QUEUE_SIZE];
    Task* current_task;
    ucontext_t context;
} Scheduler;

void initialize_scheduler(Scheduler* scheduler);
void create_task(TaskRoutine routine);
void run_scheduler(void);
void yield(void);

#endif