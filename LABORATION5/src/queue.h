#ifndef QUEUE_H_
#define QUEUE_H_
#include <stdlib.h>
#include <stdio.h>

#define ERROR_NULL -1
#define ERROR_FULL -1

typedef char datatype;
typedef struct fifo_queue *Queue;

struct fifo_queue {
    datatype *arr;
    int head;
    int tail;
    size_t size;
};

Queue create_queue(size_t size);

void delete_queue(Queue q);

int enqueue(Queue q, datatype c);

datatype dequeue(Queue q);

void printQueue(Queue q);

#endif