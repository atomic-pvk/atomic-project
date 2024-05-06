#ifndef UDP_ECHO_CLIENT_DEMO_H
#define UDP_ECHO_CLIENT_DEMO_H

#include "NTP_TDMG.h"

/*
 * Create the UDP echo client tasks.  This is the version where an echo request
 * is made from the same task that listens for the echo reply.
 */
void vStartNTPClientTasks_SingleTasks(uint16_t usTaskStackSize, UBaseType_t uxTaskPriority);

extern uint32_t NTP1_server_IP;
extern Socket_t xSocket;
extern struct freertos_sockaddr xDestinationAddress;
extern Assoc_table *assoc_table;
extern struct ntp_s s;
extern struct ntp_c c;

extern TickType_t lastTimeStampTick;
extern tstamp lastTimeStampTstamp;

#endif /* UDP_ECHO_CLIENT_DEMO_H */
