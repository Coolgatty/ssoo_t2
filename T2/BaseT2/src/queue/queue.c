#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include "queue.h"

PArray* process_array_init(size_t size) 
{
  PArray* process_array = malloc(sizeof(PArray));
  process_array->array = malloc(sizeof(Process*) * size);
  process_array->used = 0;
  process_array->size = size;
  process_array->start = 0;
  return process_array;
}

void process_array_add(PArray* array, Process* node)
{
  if (array->used == array->size) 
  {
    array->size *= 2;
    array->array = realloc(array->array, array->size * sizeof(Process*));
    //printf("\033[0;33mexpanded array to size %li\033[0m\n", array->size);
  }
  array->array[array->start + array->used++] = node;
  //printf("process added at %li\n", array->start + array->used - 1);
}

Process* process_array_pop(PArray* array)
{
    Process* process = array->array[array->start];
    array->array[array->start] = NULL;
    array->start++;
    array->used--;
    if(array->start > array->size/4 && array->size > 64)
    {
        for(int i = 0; i < array->used; i++)
        {
            array->array[i] = array->array[array->start + i];
            array->array[array->start + i] = NULL;
        }
        //printf("\033[0;32mshifted array to index 0 at size %li from start index %i\033[0m\n", array->size, array->start);
        array->start = 0;
        //printf("start: %i, %li/%li\n", array->start, array->used, array->size);
    }
    return process;
}

Process* process_array_pop_with_index(PArray* array, int index)
{
    Process* process = array->array[array->start + index];
    array->array[array->start + index] = NULL;
    array->used--;
    for(int i = 0; i < array->used - index; i++)
    {
        array->array[array->start + index + i] = array->array[array->start + index + i + 1];
    }
    array->array[array->start + array->used] = NULL;
    //printf("\033[0;32mshifted array to fill the gap at index %i\033[0m\n", array->start + index);
    //printf("start: %i, %li/%li\n", array->start, array->used, array->size);
    return process;
}

void process_array_destroy(PArray* array)
{
  free(array->array);
  free(array);
}

void process_array_destroy_recursive(PArray* array)
{
  for(int i = 0; i < array->used; i++)
  {
    Process* process = array->array[array->start + i];
    process_destroy(process);
  }
  free(array->array);
  free(array);
}

Queue* queue_init(int priority, int q)
{
    Queue* queue = malloc(sizeof(Queue));
    if(priority == 1 || priority == 2)
    {
        queue->type = FIFO;
    }
    else
    {
        queue->type = SFJ;
    }
    queue->quantum = priority*q;
    PArray* process_array = process_array_init(2);
    queue->queue = process_array;
    return queue;
}

void queue_destroy(Queue* queue)
{
    process_array_destroy(queue->queue);
    free(queue);
}

MLFQ* mlfq_init(int q)
{
    MLFQ* mlfq = malloc(sizeof(MLFQ));
    mlfq->firstQueue = queue_init(2, q);
    mlfq->secondQueue = queue_init(1, q);
    mlfq->thirdQueue = queue_init(0, q);
    return mlfq;
}

int mlfq_get_quantum(MLFQ* mlfq, int priority)
{
    int quantum;
    if(priority == 0)
    {
        quantum = -1;
    }
    if(priority == 1)
    {
        quantum = mlfq->secondQueue->quantum;
    }
    if(priority == 2)
    {
        quantum = mlfq->firstQueue->quantum;
    }
    return quantum;
}

void mlfq_add_process(MLFQ* mlfq, Process* process, int priority)
{
    if(priority == 0)
    {
        process_array_add(mlfq->thirdQueue->queue, process);
    }
    else if(priority == 1)
    {
        process_array_add(mlfq->secondQueue->queue, process);
    }
    else if(priority == 2)
    {
        process_array_add(mlfq->firstQueue->queue, process);
    }
    
}

int min(int a, int b)
{
    if(a <= b)
    {
        return a;
    }
    else
    {
        return b;
    }
    
}

int mlfq_cmpr(const void* a, const void* b) 
{
    // a is a pointer into the array of pointers
    Process* left = *(Process**)a;
    Process* right = *(Process**)b;

    int timeA = min(left->cycles - left->running_time, left->wait - (left->running_time % left->wait));
    int timeB = min(right->cycles - right->running_time, right->wait - (right->running_time % right->wait));

    if ( timeA < timeB)
	{
        return -1;
	}
	else if (timeA > timeB)
    {
	    return 1;
	}
	else
    {
		return 0;
	}   
}

Process* mlfq_get_process(MLFQ* mlfq)
{
    Process* process = NULL;
    if(mlfq->firstQueue->queue->used)
    {
        int i = 0;
        while(mlfq->firstQueue->queue->array[i]->state != READY)
        {
            i++;
            if(i >= mlfq->firstQueue->queue->used)
            {
                break;
            }
        }
        if(i < mlfq->firstQueue->queue->used)
        {
            process = process_array_pop_with_index(mlfq->firstQueue->queue, i);
        }
    }
    
    if(!process && mlfq->secondQueue->queue->used)
    {
        int i = 0;
        while(mlfq->secondQueue->queue->array[i]->state != READY)
        {
            i++;
            if(i >= mlfq->secondQueue->queue->used)
            {
                break;
            }
        }
        if(i < mlfq->secondQueue->queue->used)
        {
            process = process_array_pop_with_index(mlfq->secondQueue->queue, i);
        }
    }

    if(!process && mlfq->thirdQueue->queue->used)
    {
        qsort(mlfq->thirdQueue->queue->array, mlfq->thirdQueue->queue->used, sizeof(Process*), mlfq_cmpr);
        int i = 0;
        while(mlfq->thirdQueue->queue->array[i]->state != READY)
        {
            i++;
            if(i >= mlfq->thirdQueue->queue->used)
            {
                break;
            }
        }
        if(i < mlfq->thirdQueue->queue->used)
        {
            process = process_array_pop_with_index(mlfq->thirdQueue->queue, i);
        }
    }
    return process;
}

int mlfq_is_empty(MLFQ* mlfq)
{
    if(mlfq->firstQueue->queue->used == 0 && mlfq->secondQueue->queue->used == 0 && mlfq->thirdQueue->queue->used == 0)
    {
        return 1;
    }
    return 0;
}

void mlfq_destroy(MLFQ* mlfq)
{
    queue_destroy(mlfq->firstQueue);
    queue_destroy(mlfq->secondQueue);
    queue_destroy(mlfq->thirdQueue);
    free(mlfq);
}