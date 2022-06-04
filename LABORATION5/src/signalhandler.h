#ifndef SIGNALHANDLER_H_
#define SIGNALHANDLER_H_
#include <termios.h>

#define STDIN 0

// Global data
extern char input;
extern char traffic_lights;
extern struct termios tty;

// Initiate COM1 port
int initiate_COM1();
// Configure COM1 port
int configure_COM1();
// function for thread1 for reading Keyboard & COM1 port with select()
void *react_input( void* arg );

#endif
