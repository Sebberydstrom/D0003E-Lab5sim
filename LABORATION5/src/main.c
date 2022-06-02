#include "signalhandler.h"
#include "simulator.h"
#include "common.h"

#include <stdio.h>
#include <pthread.h>
#include <semaphore.h>
#include <time.h>


int main() {
    // Initiation
    //initiate_COM1();
    //configure_COM1();
    initiateCommons();
    // Create threads.
    pthread_t t1, t2, t3, t4, t5;
    pthread_create(&t1, NULL, react_input, NULL);
    pthread_create(&t2, NULL, handle_arrivals, NULL);
    pthread_create(&t3, NULL, handle_trafficlights, NULL);
    pthread_create(&t4, NULL, handle_bridgeSouth, NULL);
    pthread_create(&t5, NULL, handle_bridgeNorth, NULL);
    // First print.
    print_data(STATUS_CHANGE);
    pthread_join(t1, NULL);

    return 0;
}