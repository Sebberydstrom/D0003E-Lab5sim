#include "simulator.h"
#include "common.h"

// C library headers
#include <stdio.h>
#include <string.h>

// Linux headers
#include <unistd.h> // write(), read(), close()
#include <signal.h>
#include <time.h>



void *handle_arrivals( void* arg ) {
    while(1) {
        sem_wait(&keyinput);
        // Read last keyboard input.
        char key = get_keypress();
        // Synca över writer - gör detta i write funktionen. 
        switch(key) 
        {
            case 's':
                write_r(SOUTH_CAR_ARRIVAL);
                //write_car_data(ADD_SOUTH);
                //print_data(STATUS_CHANGE);
                break;
            case 'n':
                write_r(NORTH_CAR_ARRIVAL);
                //write_car_data(ADD_NORTH);
                //print_data(STATUS_CHANGE);
                break;
            case 'e':
                print_data(EXIT_NOW);
                exit(0);
            default:
                continue;
        }
    }
}


void *handle_trafficlights( void* arg ) {

    while(1) {

        Lights lightData = read_light_data();
        Cars carData = read_car_data();
        if (lightData.northGreen && lightData.southRed && (carData.northQueue > 0)) {
            // Ok to drive from northern direction.
            add_active_cars();
            write_car_data(SUB_NORTH);
            write_car_data(ADD_NORTH_ATBRIDGE);
            write_r(NORTH_BRIDGE_ENTRY);
            // Calculate future timestamp for a car to leave bridge.
            calculate_and_add_future_time();
            // Add which Queue the car on the bridge belong to.
            add_whichQData(NORTH_QUEUE);
            sleep(1);
            //sem_post(&north_bridge);
        }
        else if (lightData.northRed && lightData.southGreen && (carData.southQueue > 0)) {
            // Ok to drive from southern direction.
            add_active_cars();
            write_car_data(SUB_SOUTH);
            write_car_data(ADD_SOUTH_ATBRIDGE);
            write_r(SOUTH_BRIDGE_ENTRY);
            // Calculate future timestamp for a car to leave bridge.
            calculate_and_add_future_time();
            // Add which Queue the car on the bridge belong to.
            add_whichQData(SOUTH_QUEUE);
            sleep(1);
            //sem_post(&south_bridge);
        }
        else if (((lightData.northRed && lightData.southRed) && (redlights == true))) {
            // Both are red: default set.
            redlights = false;
            print_data(STATUS_CHANGE);
        }

        if (get_active_cars() == 1) {
            // Activate the schedule new time thread for the first time.
            sem_post(&timescheduler);
        }
    }
}

void *schedule_new_time( void * arg) {
    // Setup timer for next event.
    timer_t timer_id;
    struct sigevent ev;
    ev.sigev_notify = SIGEV_SIGNAL;
    ev.sigev_signo = SIGALRM;
    ev.sigev_value.sival_ptr = &timer_id;
    
    if (timer_create(CLOCK_REALTIME, &ev, &timer_id) == -1) {
        perror("timer_create");
        exit(0);
    }
    // Need to fetch latest added time and calculate the diff from current time, and schedule a signal to
    // signal the sig_handler(). 
    while(1) {
        sem_wait(&timescheduler);
        // Get the current time.
        struct timespec ts;
        if (clock_gettime(CLOCK_REALTIME, &ts) == -1) {
            perror("clock_gettime");
            exit(0);
        }
        // Get future time.
        int future_time = get_timeData();
        if (future_time == -1) {
            set_active_cars_zero();
        }
        time_t curr_time = ts.tv_sec;
        time_t next_run = curr_time - future_time;
        // fetch the latest future time to schedule.
        if(future_time != -1 && next_run > 0) {
            struct itimerspec spec;
            spec.it_interval.tv_nsec = 0;
            spec.it_interval.tv_sec = 0;
            spec.it_value.tv_nsec = 0;
            spec.it_value.tv_sec = next_run;

            if (timer_settime(timer_id, 0, &spec, NULL) == -1) {
                perror("timer_settime");
                exit(0);
            }
        } else if( future_time == -1) {
            set_active_cars_zero();
        }

    }
}

void *update_lightdata( void* arg) {
    while(1) {
        // Reads and sets the correct "latest actual" traffic data.
        sem_wait(&set_trafficlights);
        char traffic_lights = get_lightData();
        write_light_data(traffic_lights);
        // print the update of the light change, might keep this or remove it depening on if
        // it looks better.
        print_data(STATUS_CHANGE);
    }
}


void *handle_bridge_exit( void* arg) {
    while(1) {
        sem_wait(&bridge_exit);
        int identifier = get_whichQData();
        if(identifier == NORTH_QUEUE) {
            write_car_data(SUB_NORTH_ATBRIDGE);
            print_data(STATUS_CHANGE);
        } 
        else if(identifier == SOUTH_QUEUE) {
            write_car_data(SUB_SOUTH_ATBRIDGE);
            print_data(STATUS_CHANGE);
        }
    }
}

void *handle_bridgeSouth( void* arg ) {
    while(1) {
        sem_wait(&south_bridge);
        sleep(5);
        write_car_data(SUB_SOUTH_ATBRIDGE);
        print_data(STATUS_CHANGE);
    }
}


void *handle_bridgeNorth( void* arg ) {
    while(1) {
        sem_wait(&north_bridge);
        sleep(5);
        write_car_data(SUB_NORTH_ATBRIDGE);
        print_data(STATUS_CHANGE);
    }
}

void calculate_and_add_future_time() {
    struct timespec ts;
    if (clock_gettime(CLOCK_REALTIME, &ts) == -1) {
        perror("clock_gettime");
        exit(0);
    }
    // Add the time a car that has entered the bridge will need to exit.
    int future_time = ts.tv_sec + 5;
    // Add to queue timeQueue.
    add_timedata(future_time);
}






        // 2. if (n-green & s-red) { - Sätter endast data till en trafikljus char för stunden.
                // - Bilar från norra kön får börja köra
                // - 1 sec delay för varje bil att anlända till bron
                // - Efter 1 sec gör följande: 1 sec delay för denna tråd. - detta är en cyklisk tråd.
                //      - North queue - 1. -> mutex
                //      - On bridge queue + 1. -> mutex
                //      - write to COM1 port - Bit 1: Northbound bridge entry sensor activated. -> 
                //      - print to stdout the new status. - skicka signal till tråden för stdout?
                //      - skicka signal till bridge queue tråden att ta bort en bil efter 5 sec. (Anv semaphore.)
                //      - then print to stdout the new status - skicka signal till tråden för stdout?

                // - 5 sec delay för varje bil att köra över bron
            //} 
        // 3. if (n-red & s-green) - sätter endast data till en trafikljus struct.
            // - Bilar från södra kön får börja köra
            // - 1 sec delay för varje bil att anlända till bron
            // - Efter 1 sec gör följande: 1 sec delay för denna tråd.
            //      - South queue - 1
            //      - on bridge queue + 1
            //      - Write to COM1 port - Bit 2: Southbound car arrival sensor activated
            //      - print to stdout the new status.
            //      - skicka signal till bridge queue tråden att ta bort en bil efter 5 sec. (Anv sempahor)
            //      - then print to stdout the new status, how to signal stdout? by setting mutex for a enum data struct.

        // 4. if (n-red & s-red) - Inga bilar får köra - default läge?
            //      - print to stdout the status - ändra inget.

        // 5. else (error).
            //      - print error msg to stdout.

