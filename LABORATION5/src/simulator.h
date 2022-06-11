#ifndef SIMULATOR_H_
#define SIMULATOR_H_

void *handle_arrivals( void* arg );
void *handle_trafficlights( void* arg );
void *schedule_new_time( void * arg);
void *update_lightdata( void* arg);
void *handle_bridge_exit( void* arg);
void *handle_bridgeSouth( void* arg );
void *handle_bridgeNorth( void* arg );
void calculate_and_add_future_time();

#endif