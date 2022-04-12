#pragma once

#ifndef PROCESS_H
#define PROCESS_H

#include <time.h>

enum pState {READY, RUNNING, WAITING, FINISHED};

typedef struct process_data
{
    int executed;
    int interrupted;
    int turnaround;
    int response;
    int waiting_time;
} PData;

PData* pdata_init();

typedef struct process
{
    int pid;
    int startTime;
    int maxAge;
    int cycles;
    int wait;
    int waiting_delay;
    enum pState state;
    int priority;
    int running_time;
    int waiting_start;
    PData* pdata;
    char* name;
} Process;

Process* process_init(char* name, int pid, int startTime, int cycles, int wait, int waiting_delay, int s);

void process_destroy(Process* process);

#endif