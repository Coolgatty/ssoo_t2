#pragma once


#ifndef QUEUE_H
#define QUEUE_H

#include "../process/process.h"

enum queueType {FIFO, SFJ};

typedef struct process_array
{
  int start;
  size_t used;
  size_t size;
  Process** array;
} PArray;

PArray* process_array_init(size_t size);
void process_array_add(PArray* array, Process* node);
Process* process_array_pop(PArray* array);
Process* process_array_pop_with_index(PArray* array, int index);
void process_array_destroy(PArray* array);
void process_array_destroy_recursive(PArray* array);

typedef struct queue
{
    int priotity;
    int quantum;
    enum queueType type;
    PArray* queue;
} Queue;

Queue* queue_init(int priority, int q);
void queue_destroy(Queue* queue);

typedef struct mlfq
{
    Queue* firstQueue;
    Queue* secondQueue;
    Queue* thirdQueue;
    
} MLFQ;


MLFQ* mlfq_init(int q);
int mlfq_get_quantum(MLFQ* mlfq, int priority);
void mlfq_add_process(MLFQ* mlfq, Process* process, int priority);
int min(int a, int b);
int mlfq_cmpr(const void* a, const void* b);
Process* mlfq_get_process(MLFQ* mlfq);
int mlfq_is_empty(MLFQ* mlfq);
void mlfq_destroy(MLFQ* mlfq);

#endif