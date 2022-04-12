#pragma once


#ifndef SCHEDULER_H
#define SCHEDULER_H

#include "../queue/queue.h"

typedef struct scheduler
{
    PArray* processes;
    int quantum;
    int startTime;
} Scheduler;


Scheduler* scheduler_init();
void scheduler_destroy(Scheduler* scheduler);

#endif