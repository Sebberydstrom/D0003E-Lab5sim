#include "simulator.h"
#include "common.h"

// C library headers
#include <stdio.h>
#include <string.h>

// Linux headers
#include <unistd.h> // write(), read(), close()



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
        // Kan behöva sätta en liten tid i delay här, så att trafikljus signalen från AVR hinner 
        // Anlända och läsas in. Synca med semaphor.
        //Lights traffic = read_light_data();       -> not necessary anymore? only one thread that write and read light data struct.
        // Set latest traffic light data.
        write_light_data(get_lightData());
        Cars carData = read_car_data();
        if (lights.northGreen && lights.southRed && (carData.northQueue > 0)) {
            // Ok to drive from northern direction.
            write_car_data(SUB_NORTH);
            print_data(STATUS_CHANGE);
            sleep(1);
            write_car_data(ADD_NORTH_ATBRIDGE);
            write_r(NORTH_BRIDGE_ENTRY);
            sem_post(&north_bridge);
        }
        else if (lights.northRed && lights.southGreen && (carData.southQueue > 0)) {
            // Ok to drive from southern direction.
            write_car_data(SUB_SOUTH);
            print_data(STATUS_CHANGE);
            sleep(1);
            write_car_data(ADD_SOUTH_ATBRIDGE);
            write_r(SOUTH_BRIDGE_ENTRY);
            sem_post(&south_bridge);
        }
        else if ((!(lights.northRed && lights.southRed) && (redlights == true))) {
            // Both are red: default set.
            redlights = false;
            print_data(STATUS_CHANGE);
        }
        else {
            continue;
        }
        //else {
        //    // Make sure you print the right error information.
        //    print_data(STATUS_CHANGE);
        //    print_data(ERROR);
        //    sleep(5);
        //}
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

