#include "signalhandler.h"
#include "simulator.h"
#include "common.h"

#include <stdio.h>
#include <pthread.h>
#include <semaphore.h>
#include <time.h>
#include <signal.h>


// Timer handler.
void handle_signal(int signo) {
    // signal thread for exiting cars from bridge through semaphore
    sem_post(&bridge_exit);
    // signal thread for scheduling new times.
    sem_post(&timescheduler);
}

// Register the timer handler.
struct sigaction act;
void register_handler() {
    act.sa_handler = handle_signal;
    act.sa_flags = 0;

    // signals to block while running
    if (sigemptyset(&act.sa_mask) == -1) {
        perror("sigaction");
        exit(EXIT_FAILURE);
    }

    if (sigaction(SIGALRM, &act, NULL) == -1) {
        perror("sigaction");
        exit(EXIT_FAILURE);
    }
}

int main() {
    // Initiation
    initiateCommons();
    //initiate_COM1();
    //configure_COM1();

    // Setup timer for next event.
    register_handler();
    
    // Create threads.
    pthread_t t1, t2, t3, t4, t5, t6;
    pthread_create(&t1, NULL, react_input, NULL);
    pthread_create(&t2, NULL, handle_arrivals, NULL);
    pthread_create(&t3, NULL, handle_trafficlights, NULL);
    pthread_create(&t4, NULL, update_lightdata, NULL);
    pthread_create(&t5, NULL, handle_bridge_exit, NULL);
    pthread_create(&t6, NULL, schedule_new_time, NULL);
   // pthread_create(&t5, NULL, handle_bridgeSouth, NULL);
   // pthread_create(&t6, NULL, handle_bridgeNorth, NULL);
    // First print.
    print_data(STATUS_CHANGE);
    pthread_join(t1, NULL);

    return 0;
}