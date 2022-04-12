#include <stdio.h>
#include <stdlib.h>
#include "scheduler.h"

Scheduler* scheduler_init()
{
    Scheduler* scheduler = malloc(sizeof(Scheduler));
    scheduler->processes = process_array_init(2);
    scheduler->quantum = -1;
    scheduler->startTime = -1;
    return scheduler;
}

void scheduler_destroy(Scheduler* scheduler)
{
    process_array_destroy_recursive(scheduler->processes);
    free(scheduler);
}