#include "queue.h"


/* FIFO queue implementation in C using circular array */


Queue create_queue(size_t size) {
    Queue q = malloc(sizeof(Queue));

    q->arr = malloc(sizeof(datatype) * size);
    q->head = 0;
    q->tail = 0;
    q->size = size;

    return q;
}

void delete_queue(Queue q) {
    free(q->arr);
    free(q);
}

int enqueue(Queue q, datatype c) {
    if ((q->head + 1) % q->size == q->tail) {
        q->arr[q->head] = c;
        return ERROR_FULL;
    }
    q->arr[q->head] = c;
    q->head = (q->head + 1) % q->size;
    return 0;
}

datatype dequeue(Queue q) {
    if (q->head == q->tail) {
        // Always return the latest added value, until a new one has arrived.
        datatype c = q->arr[q->tail];
        return c;
    }
    datatype c = q->arr[q->tail];
    q->tail = (q->tail + 1) % q->size;

    return c;
}

void printQueue(Queue q) {
    for(int i = 0; i<q->size; i++) {
        printf("element: %d, char: %c\n", i, q->arr[i]);
    }
}