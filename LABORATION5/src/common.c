#include "common.h"
#include <unistd.h> // write(), read(), close()

// Happens from main.
void initiateCommons() {
    // Initiate queues and data.
    lightchars = create_queue(QUEUE_SIZE);
    keychars = create_queue(QUEUE_SIZE);
    cars = (Cars) { .northQueue = 0, .nAtBridge = 0, .southQueue = 0, .sAtBridge = 0 };
    lights = (Lights) { .northGreen = false, .northRed = false, .southGreen = false, .southRed = false };
    redlights = true;

    // Initiate semaphores and mutex:s. 
    pthread_mutex_init(&lock_lights, NULL);
    pthread_mutex_init(&lock_cars, NULL);
    pthread_mutex_init(&write_lock, NULL);
    pthread_mutex_init(&keycharlock, NULL);
    pthread_mutex_init(&lightcharlock, NULL);
    pthread_mutex_init(&lock_stdout, NULL);
    sem_init(&keyinput, 0, 0);
    sem_init(&north_bridge, 0, 0);
    sem_init(&south_bridge, 0, 0);

    // Initiate output string.
    simoutput = 
        "[Simulator]\n"
        "   Northbound:\n"
        "       In Queue:   %d\n"
        "       At bridge:  %d\n"
        "   Southbound:\n"
        "       In Queue:   %d\n"
        "       At bridge:  %d\n\n"
        "[Lights]\n"
        "   Northbound:\n"
        "       Green:  %s\n"
        "       Red:    %s\n"
        "   Southbound:\n"
        "       Green:  %s\n"
        "       Red:    %s\n\n"
        "Input: Add car to north queue (n), Add car to south queue (s), Exit (e):\n\n";
}

void add_keypress(char c) {
    pthread_mutex_lock(&keycharlock);
    if (enqueue(keychars, c) == -1) {
        print_data(KEYCHAR_FULL);
    };
    pthread_mutex_unlock(&keycharlock);
}

char get_keypress() {

    char lastinput;
    pthread_mutex_lock(&keycharlock);
    lastinput = dequeue(keychars);
    if ( lastinput == -1) {
        print_data(KEYCHAR_NULL);
        lastinput = 0;
    };
    pthread_mutex_unlock(&keycharlock);

    return lastinput;
}

void add_lightData(char l) {
    pthread_mutex_lock(&lightcharlock);
    if (enqueue(lightchars, l) == -1) {
        print_data(LIGHTCHAR_FULL);
    };
    pthread_mutex_lock(&lightcharlock);
}

char get_lightData() {
    char lastinput;
    pthread_mutex_lock(&lightcharlock);
    lastinput = dequeue(lightchars);
    if ( lastinput == -1) {
        print_data(LIGHTCHAR_NULL);
        lastinput = 0;
    };
    pthread_mutex_lock(&lightcharlock);
    return lastinput;
}

void print_data(int flag) {

    pthread_mutex_lock(&lock_stdout);
    if (flag == KEYCHAR_FULL) {
        printf("write error: the (keychar) queue is full, you cant store more characters.\n");
    }
    else if (flag == KEYCHAR_NULL) {
        printf("read error: you have read all the stored characters. \n");
    }
    else if (flag == LIGHTCHAR_FULL) {
        printf("write error: the (ligthchar) queue is full, you cant store more characters\n");
    }
    else if (flag == LIGHTCHAR_NULL) {
        printf("read error: you have read all the stored characters. (lightchar)\n");
    }
    else if (flag == EXIT_NOW) {
        printf("You pressed e, exits the program.\n");
    }
    else if (flag == STATUS_CHANGE) {
        // 1. Fetch necessary data.
        Cars carData = read_car_data();
        Lights trafficData = read_light_data();
        // 2. print the status.
        printf(
            simoutput,
            carData.northQueue,
            carData.nAtBridge,
            carData.southQueue,
            carData.sAtBridge,
            trafficData.northGreen ? "YES" : "NO",
            trafficData.northRed ? "YES" : "NO",
            trafficData.southGreen ? "YES" : "NO",
            trafficData.southRed ? "YES" : "NO"
            );
    }
    else if (flag == ERROR) {
        printf("Error, possibly green lights at both sides or No more cars in queues.\n");
    }
    pthread_mutex_unlock(&lock_stdout);
}

void write_r(char ctrl) {
    pthread_mutex_lock(&write_lock);
    unsigned char msg;

    if (ctrl == SOUTH_CAR_ARRIVAL) {
        write_car_data(ADD_SOUTH);
        msg = (1 << SOUTH_CAR_ARRIVAL);
        if (write(COM1, &msg, sizeof(msg)) == -1) {
            perror("write car arrival data");
            exit(0);
        };
        print_data(STATUS_CHANGE);

    }
    else if (ctrl == NORTH_CAR_ARRIVAL) {
        write_car_data(ADD_NORTH);
        msg = (1 << NORTH_CAR_ARRIVAL);
        if (write(COM1, &msg, sizeof(msg)) == -1) {
            perror("write car arrival data");
            exit(0);
        };
        print_data(STATUS_CHANGE);
    } 
    else if (ctrl == NORTH_BRIDGE_ENTRY) {
        msg = (1 << NORTH_BRIDGE_ENTRY);
        if (write(COM1, &msg, sizeof(msg)) == -1) {
            perror("write north bridge entry data");
            exit(0);
        }
        print_data(STATUS_CHANGE);

    }
    else if (ctrl == SOUTH_BRIDGE_ENTRY) {
        msg = (1 << SOUTH_BRIDGE_ENTRY);
        if (write(COM1, &msg, sizeof(msg)) == -1) {
            perror("write south bridge entry data");
            exit(0);
        }
        print_data(STATUS_CHANGE);

    }
    pthread_mutex_unlock(&write_lock);
}


void write_car_data(char ctrl) {
    pthread_mutex_lock(&lock_cars);
    if (ctrl == ADD_SOUTH) {
        cars.southQueue = cars.southQueue + 1;
    }
    else if (ctrl == SUB_SOUTH) {
        cars.southQueue = cars.southQueue - 1;
    }
    else if (ctrl == ADD_SOUTH_ATBRIDGE) {
        cars.sAtBridge = cars.sAtBridge + 1;
    }
    else if (ctrl == SUB_SOUTH_ATBRIDGE) {
        cars.sAtBridge = cars.sAtBridge - 1;
    }
    else if (ctrl == ADD_NORTH) {
        cars.northQueue = cars.northQueue + 1;
    }
    else if (ctrl == SUB_NORTH) {
        cars.northQueue = cars.northQueue - 1;
    }
    else if (ctrl == ADD_NORTH_ATBRIDGE) {
        cars.nAtBridge = cars.nAtBridge + 1;
    }
    else if (ctrl == SUB_NORTH_ATBRIDGE) {
        cars.nAtBridge = cars.nAtBridge - 1;
    }
    else if (ctrl == SUB_NQUEUE_ADD_NBRIDGE) {
        cars.northQueue = cars.northQueue - 1;
        cars.nAtBridge = cars.nAtBridge + 1;
    }
    else if (ctrl == SUB_SQUEUE_ADD_SBRIDGE) {
        cars.southQueue = cars.southQueue - 1;
        cars.sAtBridge = cars.sAtBridge + 1;
    }
    pthread_mutex_unlock(&lock_cars);
}

Cars read_car_data() {
    Cars c;
    pthread_mutex_lock(&lock_cars);
    c = cars;
    pthread_mutex_unlock(&lock_cars);
    return c;
}

void write_light_data(char light_status) {
    // Enqueue till fifoqueue här.
    pthread_mutex_lock(&lock_lights);
    // Bit 0 - Northbound green light
    if (light_status & (1 << NORTH_GREEN)) {
        lights.northGreen = true;
    } else {
        lights.northGreen = false;
    }
    // Bit 1 - Northbound red light
    if (light_status & (1 << NORTH_RED)) {
        lights.northRed = true;
    } else {
        lights.northRed = false;
    }
    // Bit 2 - Southbound green light
    if (light_status & (1 << SOUTH_GREEN)) {
        lights.southGreen = true;
    } else {
        lights.southGreen = false;
    }
    // Bit 3 - Southbound red light
    if (light_status & (1 << SOUTH_RED)) {
        lights.southRed = true;
    } else {
        lights.southRed = false;
    }
    pthread_mutex_unlock(&lock_lights);
}

Lights read_light_data() {
    Lights l;
    pthread_mutex_lock(&lock_lights);
    l = lights;
    pthread_mutex_unlock(&lock_lights);
    return l;
}