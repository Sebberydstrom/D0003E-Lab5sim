#ifndef OTHERQUEUE_H_
#define OTHERQUEUE_H_
#include <stdlib.h>
#include <stdio.h>

#define ERROR_N -1
#define ERROR_F -1

typedef int integerType;
typedef struct other_fifo_queue *otherQueue;

struct other_fifo_queue {
    integerType *arr;
    int head;
    int tail;
    size_t size;
};

otherQueue create_queue_other(size_t size);

void delete_queue_other(otherQueue q);

int enqueue_other(otherQueue q, integerType c);

integerType dequeue_other(otherQueue q);

void printQueue_other(otherQueue q);

#endif