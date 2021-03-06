#ifndef COMMON_H_
#define COMMON_H_

#include "otherqueue.h"
#include "queue.h"
#include <pthread.h>
#include <semaphore.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>

// Flags for which carQueue
#define NORTH_QUEUE 0
#define SOUTH_QUEUE 1

// Flags for STDOUT.
#define KEYCHAR_NULL   0
#define KEYCHAR_FULL   1
#define LIGHTCHAR_NULL 2
#define LIGHTCHAR_FULL 3
#define STATUS_CHANGE  4
#define EXIT_NOW       5
#define ERROR          6
#define BAD_INPUT      7

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
#define QUEUE_SIZE_OTHER 1000


// Semaphores and mutex.
extern pthread_mutex_t lock_lights;
extern pthread_mutex_t lock_cars;
extern pthread_mutex_t write_lock;
extern pthread_mutex_t keycharlock;
extern pthread_mutex_t lightcharlock;
extern pthread_mutex_t lock_stdout;
extern pthread_mutex_t lock_activecars;
extern pthread_mutex_t timelock;
extern pthread_mutex_t whichQlock;
extern sem_t set_trafficlights;
extern sem_t keyinput;
extern sem_t bridge_exit;
extern sem_t timescheduler;
extern sem_t north_bridge;
extern sem_t south_bridge;

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
extern Queue lightchars;
extern Queue keychars;
extern otherQueue timeQueue;
extern otherQueue whichQueue;
extern Cars cars;
extern Lights lights;
extern int COM1;
extern int first_car_entry;
extern const char* simoutput;
extern bool redlights;
extern bool has_added_data_from_avr;

// Common methods
void initiateCommons();
void add_keypress(char c);
char get_keypress();
void print_data(int flag);
void add_lightData(char l);
char get_lightData();
void write_r(char ctrl);
void write_car_data(char ctrl);
Cars read_car_data();
void write_light_data(char ctrl);
Lights read_light_data();
void add_active_cars();
void set_active_cars_zero();
int get_active_cars();
void add_timedata(int t);
int get_timeData();
void add_whichQData(int identifier);
int get_whichQData();



#endif