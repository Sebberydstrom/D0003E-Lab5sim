#include "common.h"
#include <unistd.h> // write(), read(), close()

/* Global definitions */
// Semaphores and mutex
pthread_mutex_t lock_lights;
pthread_mutex_t lock_cars;
pthread_mutex_t write_lock;
pthread_mutex_t keycharlock;
pthread_mutex_t lightcharlock;
pthread_mutex_t lock_stdout;
pthread_mutex_t lock_activecars;
pthread_mutex_t timelock;
pthread_mutex_t whichQlock;
sem_t set_trafficlights;
sem_t keyinput;
sem_t north_bridge;
sem_t south_bridge;
sem_t bridge_exit;
sem_t timescheduler;

// Globals
Queue lightchars;
Queue keychars;
otherQueue timeQueue;
otherQueue whichQueue;
Cars cars;
Lights lights;
int COM1;
int first_car_entry;
bool redlights;
bool has_added_data_from_avr;
const char* simoutput =
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

// Happens from main.
void initiateCommons() {
    // Initiate queues and data.
    lightchars = create_queue(QUEUE_SIZE);
    keychars = create_queue(QUEUE_SIZE);
    timeQueue = create_queue_other(QUEUE_SIZE_OTHER);
    whichQueue = create_queue_other(QUEUE_SIZE_OTHER);
    cars = (Cars) { .northQueue = 0, .nAtBridge = 0, .southQueue = 0, .sAtBridge = 0 };
    lights = (Lights) { .northGreen = false, .northRed = false, .southGreen = false, .southRed = false };
    redlights = true;
    has_added_data_from_avr = false;
    first_car_entry = 0;
    COM1 = -1;

    // Initiate semaphores and mutex:s. 
    pthread_mutex_init(&lock_lights, NULL);
    pthread_mutex_init(&lock_cars, NULL);
    pthread_mutex_init(&write_lock, NULL);
    pthread_mutex_init(&keycharlock, NULL);
    pthread_mutex_init(&lightcharlock, NULL);
    pthread_mutex_init(&lock_stdout, NULL);
    pthread_mutex_init(&lock_activecars, NULL);
    pthread_mutex_init(&timelock, NULL);
    pthread_mutex_init(&whichQlock, NULL);

    sem_init(&set_trafficlights, 0, 0);
    sem_init(&keyinput, 0, 0);
    sem_init(&timescheduler, 0, 0);
    sem_init(&bridge_exit, 0, 0);
    sem_init(&north_bridge, 0, 0);
    sem_init(&south_bridge, 0, 0);

    // Initiate output string.
    // simoutput = 
    //     "[Simulator]\n"
    //     "   Northbound:\n"
    //     "       In Queue:   %d\n"
    //     "       At bridge:  %d\n"
    //     "   Southbound:\n"
    //     "       In Queue:   %d\n"
    //     "       At bridge:  %d\n\n"
    //     "[Lights]\n"
    //     "   Northbound:\n"
    //     "       Green:  %s\n"
    //     "       Red:    %s\n"
    //     "   Southbound:\n"
    //     "       Green:  %s\n"
    //     "       Red:    %s\n\n"
    //     "Input: Add car to north queue (n), Add car to south queue (s), Exit (e):\n\n";
}

// Car has entered bridge flag data

void add_active_cars() {
    pthread_mutex_lock(&lock_activecars);
    first_car_entry += 1;
    pthread_mutex_unlock(&lock_activecars);
}

void set_active_cars_zero() {
    pthread_mutex_lock(&lock_activecars);
    first_car_entry = 0;
    pthread_mutex_unlock(&lock_activecars);
}

// OBS h??r ??r felet -> ??ndra detta imorgon.
int get_active_cars() {
    int no_cars;
    pthread_mutex_lock(&lock_activecars);
    no_cars = first_car_entry;
    pthread_mutex_unlock(&lock_activecars);
    return no_cars;
}



// Keyboard
void add_keypress(char c) {
    pthread_mutex_lock(&keycharlock);
    if (enqueue(keychars, c) == -1) {
        print_data(KEYCHAR_FULL);
    };
    pthread_mutex_unlock(&keycharlock);
}
// Keyboard
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
// Trafficlights
void add_lightData(char l) {
    pthread_mutex_lock(&lightcharlock);
    has_added_data_from_avr = true;
    if (enqueue(lightchars, l) == -1) {
        print_data(LIGHTCHAR_FULL);
    };
    pthread_mutex_unlock(&lightcharlock);
}
//Trafficlights
char get_lightData() {
    char lastinput;
    pthread_mutex_lock(&lightcharlock);
    if (has_added_data_from_avr) {
        lastinput = dequeue(lightchars);
    } else {
        lastinput = 0;
    }
    pthread_mutex_unlock(&lightcharlock);
    return lastinput;
}
// Scheduler data
void add_timedata(int t) {
    pthread_mutex_lock(&timelock);
    if (enqueue_other(timeQueue, t) == -1) {
        print_data(ERROR);
    };
    pthread_mutex_unlock(&timelock);
}
// Scheduler data
int get_timeData() {
    int lastinput;
    pthread_mutex_lock(&timelock);
    lastinput = dequeue_other(timeQueue);
    pthread_mutex_unlock(&timelock);
    return lastinput;
}
// Which car queue data
void add_whichQData(int identifier) {
    pthread_mutex_lock(&whichQlock);
    if (enqueue_other(whichQueue, identifier) == -1) {
        print_data(ERROR);
    };
    pthread_mutex_unlock(&whichQlock);
}
// Which car queue data
int get_whichQData() {
    int lastinput;
    pthread_mutex_lock(&whichQlock);
    lastinput = dequeue_other(whichQueue);
    pthread_mutex_unlock(&whichQlock);
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
    else if (flag == BAD_INPUT) {
        printf("bad input: only (s), (n) and (e) is acceptable characters.\n");
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
        printf("Error, somewhere in code.\n");
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
    // Enqueue till fifoqueue h??r.
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