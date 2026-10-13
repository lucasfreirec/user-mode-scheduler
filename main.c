#include <stdio.h>
#include "scheduler.h"

void task_A(void) {
    for (int i = 1; i <= 4; i++) {
        printf("Task A [step %d]\n", i);
        yield();
    }
}

void task_B(void) {
    for (int i = 1; i <= 4; i++) {
        printf("\t-> Task B [step %d]\n", i);
        yield();
        task_A();
    }
}

void task_C(void) {
    for (int i = 1; i <= 4; i++) {
        printf("\t\t--> Task C [step %d]\n", i);
        yield();
    }
}

void task_D(void) {
    for (int i = 1; i <= 4; i++) {
        printf("\t\t\t---> Task D [step %d]\n", i);
        yield();
    }
}

void task_E(void) {
    for (int i = 1; i <= 4; i++) {
        printf("\t\t\t\t----> Task E [step %d]\n", i);
        yield();
    }
}

int main(void) {
    Scheduler my_scheduler;

    initialize_scheduler(&my_scheduler);

    printf("Creating 5 tasks (A, B, C, D, E)...\n");

    create_task(task_A);
    create_task(task_B);
    create_task(task_C);
    create_task(task_D);
    create_task(task_E);

    printf("\n--- Starting Scheduler ---\n");

    run_scheduler();

    printf("--- Execution Finished ---\n");

    return 0;
}