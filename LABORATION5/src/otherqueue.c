#include "otherqueue.h"


/* FIFO queue implementation in C using circular array */


otherQueue create_queue_other(size_t size) {
    otherQueue q = malloc(sizeof(otherQueue));

    q->arr = malloc(sizeof(integerType) * size);
    q->head = 0;
    q->tail = 0;
    q->size = size;

    return q;
}

void delete_queue_other(otherQueue q) {
    free(q->arr);
    free(q);
}

int enqueue_other(otherQueue q, integerType c) {
    if ((q->head + 1) % q->size == q->tail) {
        q->arr[q->head] = c;
        return ERROR_F;
    }
    q->arr[q->head] = c;
    q->head = (q->head + 1) % q->size;
    return 0;
}

integerType dequeue_other(otherQueue q) {
    if (q->head == q->tail) {
        return ERROR_N;
    }
    integerType c = q->arr[q->tail];
    q->tail = (q->tail + 1) % q->size;

    return c;
}

void printQueue_other(otherQueue q) {
    for(int i = 0; i<q->size; i++) {
        printf("element: %d, char: %c\n", i, q->arr[i]);
    }
}