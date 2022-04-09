#pragma once

#ifndef PROCESS_H
#define PROCESS_H

#include <time.h>

enum State {READY, RUNNING, WAITING, FINISHED};

typedef struct process
{
    unsigned int pid;
    int priority;
    State state;
    char* name;
} Process;

Process* process_init(int pid, int priority, char* name);

void process_destroy(Process* process);

#endif