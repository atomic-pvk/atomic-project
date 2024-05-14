#ifndef UDP_ECHO_CLIENT_DEMO_H
#define UDP_ECHO_CLIENT_DEMO_H

#include "NTP_TDMG.h"
#include "freq.h"

/*
 * Create the NTP client task and the clock adjust interrupt task.
 */
void vStartNTPClientTasks_SingleTasks(uint16_t usTaskStackSize, UBaseType_t uxTaskPriority);

extern SemaphoreHandle_t timeMutex;

extern uint32_t NTP1_server_IP;
extern Socket_t xSocket;
extern struct freertos_sockaddr xDestinationAddress;
extern Assoc_table *assoc_table;
extern struct ntp_s s;
extern struct ntp_c c;

#endif /* UDP_ECHO_CLIENT_DEMO_H */
