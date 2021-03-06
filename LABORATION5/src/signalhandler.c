#include "signalhandler.h"
#include "common.h"

// C library headers
#include <stdio.h>
#include <string.h>

// Linux headers
#include <fcntl.h> // Contains file controls like O_RDWR
#include <errno.h> // Error integer and strerror() function
#include <unistd.h> // write(), read(), close()
#include <sys/time.h> // needed for select
#include <sys/types.h> // needed for select

char input;
char traffic_lights;
struct termios tty;


int initiate_COM1() {

    COM1 = open("/dev/ttyS0", O_RDWR);
    // Error checking
    printf("COM1: %d\n", COM1);
    if (COM1 < 0) {
        printf("Error %i from open: %s\n", errno, strerror(errno));
        exit(1);
    }

    return 1;
}

int configure_COM1() {
    // Configuration setup for serial port
    // Read in existing settings, and handle any error.
    if (tcgetattr(COM1, &tty) != 0) {
        printf("Error %i from tcgetattr: %s\n", errno, strerror(errno));
        exit(1);
    }
    //Clear parity bit, disabling parity
    tty.c_cflag &= ~PARENB;
    // Clear stop field, only one stop bit used in communication
    tty.c_cflag &= ~CSTOPB;
    // Clear all the size bits and set to 8 bits per byte.
    tty.c_cflag &= ~CSIZE;
    tty.c_cflag |= CS8;    
    // Disable RTS/CTS hardware flow control
    tty.c_cflag &= ~CRTSCTS;
    // Turn on READ & ignore ctrl lines (CLOCAL = 1)
    tty.c_cflag |= CREAD | CLOCAL;
    // Disable canonical mode
    tty.c_lflag &= ~ICANON;
    // Disable echo, erasure and new-line echo.
    tty.c_lflag &= ~ECHO;
    tty.c_lflag &= ~ECHOE; 
    tty.c_lflag &= ~ECHONL;
    // Disable interpretation of INTR, QUIT and SUSP
    tty.c_lflag &= ~ISIG;
    // Turn off s/w flow ctrl
    tty.c_iflag &= ~(IXON | IXOFF | IXANY);
    // Disable any special handling of received bytes
    tty.c_iflag &= ~(IGNBRK|BRKINT|PARMRK|ISTRIP|INLCR|IGNCR|ICRNL);
    // Prevent special interpretation of output bytes (e.g. newline chars)
    tty.c_oflag &= ~OPOST;
    // Prevent conversion of newline to carriage return/line feed
    tty.c_oflag &= ~ONLCR;

    // Set in/out baud rate to be 9600
    cfsetispeed(&tty, B9600);
    cfsetospeed(&tty, B9600);

    // Save tty settings, also checking for error.
    if (tcsetattr(COM1, TCSANOW, &tty) != 0) {
        printf("Error %i from tcsetattr: %s\n", errno, strerror(errno));
        exit(1);
    }
    return 1;

}

void *react_input( void* arg ) {
    int fdmax = 4;
    fd_set readfds;
    FD_ZERO(&readfds);
    FD_SET(STDIN, &readfds);
    FD_SET(COM1, &readfds);

    char key_buf[BUF_LEN_KEYBOARD];
    char avr_buf[BUF_LEN_AVR];

    while(1) {
        // Thread will use select() to listen on COM1 and Keyboard.
        if (select(fdmax, &readfds, NULL, NULL, NULL) == -1) {
            perror("select");
            exit(1);
        }
        if (FD_ISSET(STDIN, &readfds)) {
            //Handle keypress
            // 1. read keyboard character.
            if (read(STDIN, key_buf, BUF_LEN_KEYBOARD) == -1) {
                perror("reading error for STDIN");
                exit(1);
            };
            // 2. add input char to char array FIFO queue -> mutex protected.
            if (key_buf[0] == 's' || key_buf[0] == 'n' || key_buf[0] == 'e') {
                add_keypress(key_buf[0]);
                // 3. Send signal through semaphore to other thread to handle arriving car to a queue.
                sem_post(&keyinput);
            } else {
                // Not sure why this one prints x2?, because STDIO also reads in enter as a char.
                print_data(BAD_INPUT);
            }

        }
        if (FD_ISSET(COM1, &readfds)) {
            // Handle AVR data. - read & write to com1 - Should not be any problem, uses two pins for sending and receiving.
            // 1. read the 8 bit data
            // Hur mkt data returnerar jag varje g??ng jag l??ser fr??n buffern? beroende p?? det s?? 
            // Beh??ver jag hantera det p?? olika s??tt.
            ssize_t b_read = read(COM1, &avr_buf, BUF_LEN_AVR);
            if (b_read == -1) {
                perror("reading error for COM1");
                exit(1);
            };
            // 2. add input to char array FIFO queue.
            add_lightData(avr_buf[0]);
            
            // 3. If both south and north are red set redlights to true.
            if (((avr_buf[0] & (1 << NORTH_RED)) && (avr_buf[0] & (1 << SOUTH_RED)))) {
                redlights = true;
            }
            sem_post(&set_trafficlights);
        }
    }
}


























            // 2. if (n-green & s-red) { - S??tter endast data till en trafikljus char f??r stunden.
                // - Bilar fr??n norra k??n f??r b??rja k??ra
                // - 1 sec delay f??r varje bil att anl??nda till bron
                // - Efter 1 sec g??r f??ljande: 1 sec delay f??r denna tr??d. - detta ??r en cyklisk tr??d.
                //      - North queue - 1. -> mutex
                //      - On bridge queue + 1. -> mutex
                //      - write to COM1 port - Bit 1: Northbound bridge entry sensor activated. -> 
                //      - print to stdout the new status. - skicka signal till tr??den f??r stdout?
                //      - skicka signal till bridge queue tr??den att ta bort en bil efter 5 sec. (Anv semaphore.)
                //      - then print to stdout the new status - skicka signal till tr??den f??r stdout?

                // - 5 sec delay f??r varje bil att k??ra ??ver bron
            //} 
            // 3. if (n-red & s-green) - s??tter endast data till en trafikljus struct.
            // - Bilar fr??n s??dra k??n f??r b??rja k??ra
            // - 1 sec delay f??r varje bil att anl??nda till bron
            // - Efter 1 sec g??r f??ljande: 1 sec delay f??r denna tr??d.
            //      - South queue - 1
            //      - on bridge queue + 1
            //      - Write to COM1 port - Bit 2: Southbound car arrival sensor activated
            //      - print to stdout the new status.
            //      - skicka signal till bridge queue tr??den att ta bort en bil efter 5 sec. (Anv sempahor)
            //      - then print to stdout the new status

            // 4. if (n-red & s-red) - Inga bilar f??r k??ra - default l??ge?
            //      - print to stdout the status - ??ndra inget.

            // 5. else (error).
            //      - print error msg to stdout.





    // 8 Bit each send or receive of data.

    /* 
    WRITE. 
        Bit 0: Northbound car arrival sensor activated
        Bit 1: Northbound bridge entry sensor activated
        Bit 2: Southbound car arrival sensor activated
        Bit 3: Southbound bridge entry sensor activated
    */

    /*
    READ.
        Bit 0: Northbound green light status
        Bit 1: Northbound red light status
        Bit 2: Southbound green light status
        Bit 3: Southbound red light status
    */

    //void *handle_arrivals( void* arg ) {
    //    sem_wait(&car_arrival);
    // Semaphor signal.
     //sem_post(&car_arrival);

        // 2. if s: enqueues car in southbound direction. 
        //      - south queue + 1
        //      - write to COM1 port - Bit 0: Northbound car arrival sensor activated.
        //      - print to stdout status change.

        // 3. if n: enques a new car in northbound direction
        //      - north queue + 1
        //      - write to COM1 port - Bit 2: Southbound car arrival sensor activated
        //      - print to stdout status change. 

        // 4. if e: exits the simulator.
        //      - print to stdout exit.
        //      - exit().


    //}

   // void *handle_trafficlights( void* arg ) {

   // }
