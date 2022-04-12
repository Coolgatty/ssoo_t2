#include <stdio.h>	// FILE, fopen, fclose, etc.
#include <stdlib.h> // malloc, calloc, free, etc
#include "../process/process.h"
#include "../queue/queue.h"
#include "../file_manager/manager.h"
#include "../queue/queue.h"
#include "scheduler.h"

// From https://stackoverflow.com/questions/23689687/sorting-an-array-of-struct-pointers-using-qsort
int cmprfnc(const void* a, const void* b) 
{
    // a is a pointer into the array of pointers
    Process* left = *(Process**)a;
    Process* right = *(Process**)b;

    if ( left->startTime < right->startTime)
	{
        return -1;
	}
	else if (left->startTime > right->startTime)
    {
	    return 1;
	}
	else
    {
		return 0;
	}   
}

int main(int argc, char const *argv[])
{
	/*Lectura del input*/
	char *file_name = (char *)argv[1];
	InputFile *input_file = read_file(file_name);

	Scheduler* scheduler = scheduler_init();

	/*Mostramos el archivo de input en consola*/
	printf("Nombre archivo: %s\n", file_name);
	printf("Cantidad de procesos: %d\n", input_file->len);
	for (int i = 0; i < input_file->len; ++i)
	{
		Process* process = process_init(input_file->lines[i][0],
										strtol(input_file->lines[i][1], NULL, 10),
										strtol(input_file->lines[i][2], NULL, 10),
										strtol(input_file->lines[i][3], NULL, 10),
										strtol(input_file->lines[i][4], NULL, 10),
										strtol(input_file->lines[i][5], NULL, 10),
										strtol(input_file->lines[i][6], NULL, 10));
		process_array_add(scheduler->processes, process);
		// for (int j = 0; j < 7; ++j)
		// {
		// 	printf("%s ", input_file->lines[i][j]);
		// }
		// prinft("\n");
	}

	input_file_destroy(input_file);
	

	qsort(scheduler->processes->array, scheduler->processes->used, sizeof(Process*), cmprfnc);
	
	printf("Procesos:\n");
	for(int i = 0; i < scheduler->processes->used; i++)
	{
		printf("Start: %i, Cycles: %i, Process pid: %i, name: %s\n", 
											scheduler->processes->array[scheduler->processes->start + i]->startTime,
											scheduler->processes->array[scheduler->processes->start + i]->cycles,
											scheduler->processes->array[scheduler->processes->start + i]->pid,
											scheduler->processes->array[scheduler->processes->start + i]->name);
	}

	int q = strtol(argv[3], NULL, 10);
	MLFQ* mlfq = mlfq_init(q);

	Process* CPU = NULL;

	int time = 0;
	while(1)
	{
		// Update processes that are WAITING to READY if they are.
		for(int i = 0; i < mlfq->firstQueue->queue->used; i++)
		{
			Process* process = mlfq->firstQueue->queue->array[i];
			if(process->state == WAITING && time - process->waiting_start == process->waiting_delay)
			{
				process->state = READY;
			}
			process->pdata->waiting_time++;
		}
		for(int i = 0; i < mlfq->secondQueue->queue->used; i++)
		{
			Process* process = mlfq->secondQueue->queue->array[i];
			if(process->state == WAITING && time - process->waiting_start == process->waiting_delay)
			{
				process->state = READY;
			}
			process->pdata->waiting_time++;
		}
		for(int i = 0; i < mlfq->thirdQueue->queue->used; i++)
		{
			Process* process = mlfq->thirdQueue->queue->array[i];
			if(process->state == WAITING && time - process->waiting_start == process->waiting_delay)
			{
				process->state = READY;
			}
			process->pdata->waiting_time++;
		}
		// Update processes that are RUNNING wherever necessary
		if(CPU)
		{
			CPU->running_time++;
			
			if(CPU->wait == time - scheduler->startTime)
			{
				CPU->state = WAITING;
				if(CPU->priority < 2)
				{
					CPU->priority++;
				}
			}
			else if(scheduler->quantum > 0)
			{
				scheduler->quantum--;
				if(CPU->priority > 0)
				{
					CPU->priority--;
				}
			}
			if((time - CPU->startTime)%CPU->maxAge == 0)
			{
				CPU->priority = 2;
			}
			if(CPU->cycles == CPU->running_time)
			{
				CPU->state = FINISHED;
				CPU->pdata->turnaround = time - CPU->startTime;
			}
		}
		// Move processes to queues in order:
		//      1) Process that exits CPU
		if(CPU)
		{
			if(CPU->state == WAITING)
			{
				CPU->waiting_start = time;
				mlfq_add_process(mlfq, CPU, CPU->priority);
				CPU = NULL;
			}
			else if(CPU->state == RUNNING && scheduler->quantum == 0)
			{
				CPU->state = READY;
				CPU->pdata->interrupted++;
				mlfq_add_process(mlfq, CPU, CPU->priority);
				CPU = NULL;
			}
			else if(CPU->state == FINISHED)
			{
				CPU = NULL;
			}
		}
		// 		2) WIP: Process in process list whose timeStart == time
		for(int i = 0; i < scheduler->processes->used; i++)
		{
			Process* process = scheduler->processes->array[i];
			if(process->startTime == time)
			{
				mlfq_add_process(mlfq, process, 2);
				//printf("process added\n");
			}
		}
		// 		3) Every process from second priority to first if (time-startTime)%S == 0
		for(int i = 0; i < mlfq->secondQueue->queue->used; i++)
		{
			Process* process = mlfq->secondQueue->queue->array[i];
			if((time - process->startTime)%process->maxAge == 0)
			{
				mlfq_add_process(mlfq, process_array_pop_with_index(mlfq->secondQueue->queue, i), 2);
				i--;
			}
		}
		// 		4) Every process from third priority to first if (time-startTime)%S == 0
		for(int i = 0; i < mlfq->thirdQueue->queue->used; i++)
		{
			Process* process = mlfq->thirdQueue->queue->array[i];
			if((time - process->startTime)%process->maxAge == 0)
			{
				mlfq_add_process(mlfq, process_array_pop_with_index(mlfq->thirdQueue->queue, i), 2);
				i--;
			}
		}
		

		// Send process to CPU if available (from READY to RUNNING)
		if(!CPU)
		{
			CPU = mlfq_get_process(mlfq);
		}
		if(CPU && CPU->state == READY)
		{
			//printf("process started at time: %i\n", time);
			if(CPU->running_time == 0)
			{
				CPU->pdata->response = time - CPU->startTime;
			}
			CPU->state = RUNNING;
			CPU->pdata->executed++;
			scheduler->quantum = mlfq_get_quantum(mlfq, CPU->priority);
			scheduler->startTime = time;
		}
		// Handle no process ready to be executed
		if(!CPU && mlfq_is_empty(mlfq) && (time > scheduler->processes->array[scheduler->processes->used - 1]->startTime))
		{
			break;
		}
		time++;
	}
	
	file_name = (char *)argv[2];
	FILE *fptr;
	fptr = fopen(file_name, "w");
	for(int i = 0; i < scheduler->processes->used; i++)
	{
		Process* process = scheduler->processes->array[i];
		PData* data = process->pdata;
		fprintf(fptr, "%s,%i,%i,%i,%i,%i\n", process->name, data->executed, data->interrupted, data->turnaround, data->response, data->waiting_time);
	}
	fclose(fptr);

	mlfq_destroy(mlfq);
	scheduler_destroy(scheduler);

	
	//printf("%s\n", CPU->name);


	// PArray* array = process_array_init(1);
	// for(int i = 0; i < 100; i++)
	// {
	// 	Process* process = process_init("a", i, 0, 0, 0, 0, 0);
	// 	process_array_add(array, process);
	// }

	// // for(int i = 0; i < 40; i++)
	// // {
	// // 	process_destroy(process_array_pop(array));
	// // }
	// process_destroy(process_array_pop_with_index(array, 45));
	// printf("done\n");
	// for(int i = 0; i < array->used; i++)
	// {
	// 	printf("%i\n", array->array[i]->pid);
	// }
	// process_array_destroy_recursive(array);
}