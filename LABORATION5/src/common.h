#ifndef COMMON_H_
#define COMMON_H_

#include "queue.h"
#include <pthread.h>
#include <semaphore.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>

// Flags for STDOUT.
#define KEYCHAR_NULL  0
#define KEYCHAR_FULL  1
#define STATUS_CHANGE 2
#define EXIT_NOW      4
#define ERROR         5

// flags for cars.
#define ADD_NORTH           0
#define SUB_NORTH           1
#define ADD_NORTH_ATBRIDGE  2
#define SUB_NORTH_ATBRIDGE  3
#define ADD_SOUTH           4
#define SUB_SOUTH           5
#define ADD_SOUTH_ATBRIDGE  6
#define SUB_SOUTH_ATBRIDGE  7

#define SUB_NQUEUE_ADD_NBRIDGE    8
#define SUB_SQUEUE_ADD_SBRIDGE    9


// Simulator to AVR 
#define NORTH_CAR_ARRIVAL  0    // Northbound car arrival sensor bit
#define NORTH_BRIDGE_ENTRY 1    // Northbound bridge entry sensor bit
#define SOUTH_CAR_ARRIVAL  2    // Southbound car arrival sensor bit
#define SOUTH_BRIDGE_ENTRY 3    // Southbound bridge entry sensor bit

// AVR to Simulator
#define NORTH_GREEN 0   // Northbound green light status bit
#define NORTH_RED   1   // Northbound red light status bit
#define SOUTH_GREEN 2   // Southbound green light status bit
#define SOUTH_RED   3   // Southbound red light status bit

#define QUEUE_SIZE 100


// Semaphores and mutex.
pthread_mutex_t lock_lights;
pthread_mutex_t lock_cars;
pthread_mutex_t write_lock;
pthread_mutex_t keycharlock;
pthread_mutex_t lock_stdout;
sem_t keyinput;
sem_t north_bridge;
sem_t south_bridge;

typedef struct {
    int northQueue;
    int nAtBridge;
    int southQueue;
    int sAtBridge;
} Cars;

typedef struct {
    bool northGreen;
    bool northRed;
    bool southGreen;
    bool southRed;
} Lights;

// Global data 
Queue keychars;
Cars cars;
Lights lights;
int COM1;
const char* simoutput;

// Common methods
void initiateCommons();
void add_keypress(char c);
char get_keypress();
void print_data(int flag);
void write_r(char ctrl);
void write_car_data(char ctrl);
Cars read_car_data();
void write_light_data(char ctrl);
Lights read_light_data();



#endif