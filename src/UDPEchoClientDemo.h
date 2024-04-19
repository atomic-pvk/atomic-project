#ifndef UDP_ECHO_CLIENT_DEMO_H
#define UDP_ECHO_CLIENT_DEMO_H

#include "FreeRTOS_IP.h"
#include "FreeRTOS_Sockets.h"

#include "NTP_TDMG.h"

/*
 * Create the UDP echo client tasks.  This is the version where an echo request
 * is made from the same task that listens for the echo reply.
 */
void vStartUDPEchoClientTasks_SingleTasks(uint16_t usTaskStackSize,
                                          UBaseType_t uxTaskPriority);

/* Global variable */
extern uint32_t NTP1_server_IP;
extern Socket_t xSocket; /* socket */
extern struct freertos_sockaddr xDestinationAddress;

#endif /* UDP_ECHO_CLIENT_DEMO_H */
