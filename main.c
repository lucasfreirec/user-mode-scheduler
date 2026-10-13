#include <stdio.h>
#include "scheduler.h"

static void task_A(void)
{
    for (int i = 1; i <= 4; i++) {
        printf("Task A [step %d]\n", i);
        task_yield();
    }
}

static void task_B(void)
{
    for (int i = 1; i <= 4; i++) {
        printf("\t-> Task B [step %d]\n", i);
        task_yield();
    }
}

static void task_C(void)
{
    for (int i = 1; i <= 4; i++) {
        printf("\t\t--> Task C [step %d]\n", i);
        task_yield();
    }
}

static void task_D(void)
{
    for (int i = 1; i <= 4; i++) {
        printf("\t\t\t---> Task D [step %d]\n", i);
        task_yield();
    }
}

static void task_E(void)
{
    for (int i = 1; i <= 4; i++) {
        printf("\t\t\t\t----> Task E [step %d]\n", i);
        task_yield();
    }
}

static const char *task_state_name(TaskState state)
{
    switch (state) {
        case TASK_UNUSED:
            return "UNUSED";

        case TASK_READY:
            return "READY";

        case TASK_RUNNING:
            return "RUNNING";

        case TASK_FINISHED:
            return "FINISHED";

        case TASK_CANCELLED:
            return "CANCELLED";

        default:
            return "UNKNOWN";
    }
}

static int check_result(
    SchedulerResult result,
    const char *operation)
{
    if (result != SCHEDULER_SUCCESS) {
        fprintf(stderr, "%s failed (error %d)\n",
                operation, result);
        return 0;
    }

    return 1;
}

int main(void)
{
    Scheduler scheduler;

    if (!check_result(
            scheduler_init(&scheduler),
            "Scheduler initialization")) {
        return 1;
    }

    int task_a_id;
    int task_b_id;
    int task_c_id;
    int task_d_id;
    int task_e_id;

    printf("Creating tasks...\n");

    if (!check_result(
            scheduler_spawn(&scheduler, task_A, &task_a_id),
            "Creating task A")) {
        scheduler_destroy(&scheduler);
        return 1;
    }

    if (!check_result(
            scheduler_spawn(&scheduler, task_B, &task_b_id),
            "Creating task B")) {
        scheduler_destroy(&scheduler);
        return 1;
    }

    if (!check_result(
            scheduler_spawn(&scheduler, task_C, &task_c_id),
            "Creating task C")) {
        scheduler_destroy(&scheduler);
        return 1;
    }

    if (!check_result(
            scheduler_spawn(&scheduler, task_D, &task_d_id),
            "Creating task D")) {
        scheduler_destroy(&scheduler);
        return 1;
    }

    if (!check_result(
            scheduler_spawn(&scheduler, task_E, &task_e_id),
            "Creating task E")) {
        scheduler_destroy(&scheduler);
        return 1;
    }

    printf("Created tasks with IDs: %d, %d, %d, %d, %d\n",
           task_a_id, task_b_id, task_c_id,
           task_d_id, task_e_id);

    /*
     * Create a task specifically to demonstrate cancellation.
     */
    int cancelled_task_id;

    if (!check_result(
            scheduler_spawn(
                &scheduler, task_A, &cancelled_task_id),
            "Creating task to cancel")) {
        scheduler_destroy(&scheduler);
        return 1;
    }

    printf("\nCancelling task %d...\n", cancelled_task_id);

    if (!check_result(
            scheduler_cancel(&scheduler, cancelled_task_id),
            "Cancelling task")) {
        scheduler_destroy(&scheduler);
        return 1;
    }

    TaskState state;

    if (!check_result(
            scheduler_get_task_state(
                &scheduler, cancelled_task_id, &state),
            "Querying cancelled task")) {
        scheduler_destroy(&scheduler);
        return 1;
    }

    printf("Task %d state: %s\n",
           cancelled_task_id,
           task_state_name(state));

    printf("\n--- Starting scheduler ---\n");

    if (!check_result(
            scheduler_run(&scheduler),
            "Running scheduler")) {
        scheduler_destroy(&scheduler);
        return 1;
    }

    printf("\n--- Final task states ---\n");

    int task_ids[] = {
        task_a_id,
        task_b_id,
        task_c_id,
        task_d_id,
        task_e_id,
        cancelled_task_id
    };

    for (size_t i = 0;
         i < sizeof(task_ids) / sizeof(task_ids[0]);
         i++) {

        if (scheduler_get_task_state(
                &scheduler, task_ids[i], &state)
            == SCHEDULER_SUCCESS) {

            printf("Task %d: %s\n",
                   task_ids[i],
                   task_state_name(state));
        }
    }

    scheduler_destroy(&scheduler);

    printf("\nScheduler destroyed.\n");

    return 0;
}