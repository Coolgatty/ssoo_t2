#include "process.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

PData* pdata_init()
{
    PData* pdata = malloc(sizeof(PData));
    pdata->executed = 0;
    pdata->interrupted = 0;
    pdata->turnaround = 0;
    pdata->response = 0;
    pdata->waiting_time = 0;
    return pdata;
}

Process* process_init(char* name, int pid, int startTime, int cycles, int wait, int waiting_delay, int s)
{
    Process* process = malloc(sizeof(Process));
    char* pname = malloc(sizeof(name));
    strcpy(pname, name);
    process->name = pname;
    process->pid = pid;
    process->startTime = startTime;
    process->cycles = cycles;
    process->wait = wait;
    process->waiting_delay = waiting_delay;
    process->maxAge = s;
    process->state = READY;
    process->priority = 2;
    process->running_time = 0;
    process->waiting_start = -1;
    PData* pdata = pdata_init();
    process->pdata = pdata;
    return process;
}

void process_destroy(Process* process)
{
    if(process)
    {
        free(process->name);
        free(process->pdata);
        free(process);
    }
    else
    {
        printf("=======================================================\n");
        printf("\033[0;31mWARNING:\033[0m You are trying to free a NULL pointer!\nLine %d in %s\n", __LINE__, __FILE__);
        printf("=======================================================\n");
    }
}