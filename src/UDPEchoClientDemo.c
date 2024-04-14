#include "UDPEchoClientDemo.h"

#include "NTP_main_utility.h"
#include "NTP_peer.h"

static void vUDPSendUsingStandardInterface(void *pvParameters);

void vStartUDPEchoClientTasks_SingleTasks(uint16_t usTaskStackSize,
                                          UBaseType_t uxTaskPriority)
{
    BaseType_t x;

    /* Create the echo client tasks. */

    xTaskCreate(vUDPSendUsingStandardInterface, /* The function that implements the task. */
                "Echo0",                        /* Just a text name for the task to aid debugging. */
                usTaskStackSize,                /* The stack size is defined in FreeRTOSIPConfig.h. */
                (void *)x,                      /* The task parameter, not used in this case. */
                uxTaskPriority,                 /* The priority assigned to the task is defined in FreeRTOSConfig.h. */
                NULL);                          /* The task handle is not used. */
}

static void vUDPSendUsingStandardInterface(void *pvParameters)
{
    struct ntp_p *p; /* peer structure pointer */
    struct ntp_r *r; /* receive packet pointesr */
    struct ntp_s s;
    struct ntp_c c;

    memset(&s, sizeof(ntp_s), 0);
    s.leap = NOSYNC;
    s.stratum = MAXSTRAT;
    s.poll = MINPOLL;
    s.precision = PRECISION;
    s.p = NULL;

    memset(&c, sizeof(ntp_c), 0);
    // TODO freq file
    // if (/* frequency file */ 0)
    // {
    //     c.freq = /* freq */ 0;
    //     rstclock(FSET, 0, 0);
    // }
    // else
    // {
    //     rstclock(NSET, 0, 0);
    // }
    c.jitter = LOG2D(s.precision);

    // while (/* mobilize configurated associations */ 1)
    // {
    p = mobilize(IPADDR, IPADDR, VERSION, MODE, KEYID,
                 P_FLAGS);
    // }

    // while (0)
    // {
    r = recv_packet();
    r->dst = get_time();
    receive(r, s, c);
    // }

    Socket_t xSocket;
    struct freertos_sockaddr xDestinationAddress;
    uint8_t cString[50];
    uint32_t ulCount = 0UL;
    const TickType_t x1000ms = 1000UL / portTICK_PERIOD_MS;

    /* Send strings to port 9000 on IP address 192.168.69.2 (replace with suitable IP at run time) */
    /* use https://github.com/FreeRTOS/FreeRTOS-Libraries-Integration-Tests/tree/main/tools/echo_server for udp echo server */
    memset(&xDestinationAddress, 0, sizeof(xDestinationAddress));
    xDestinationAddress.sin_family = FREERTOS_AF_INET4; // or FREERTOS_AF_INET6 if the destination's IP is IPv6.
    xDestinationAddress.sin_address.ulIP_IPv4 = FreeRTOS_inet_addr("192.168.69.2");
    xDestinationAddress.sin_port = FreeRTOS_htons(9000); // outgoing port
    xDestinationAddress.sin_len = (uint8_t)sizeof(xDestinationAddress);

    /* Create the socket. */
    xSocket = FreeRTOS_socket(FREERTOS_AF_INET4,   /* Used for IPv4 UDP socket. */
                                                   /* FREERTOS_AF_INET6 can be used for IPv6 UDP socket. */
                              FREERTOS_SOCK_DGRAM, /*FREERTOS_SOCK_DGRAM for UDP.*/
                              FREERTOS_IPPROTO_UDP);

    /* Check the socket was created. */
    configASSERT(xSocket != FREERTOS_INVALID_SOCKET);

    /* NOTE: FreeRTOS_bind() is not called.  This will only work if
    ipconfigALLOW_SOCKET_SEND_WITHOUT_BIND is set to 1 in FreeRTOSIPConfig.h. */

    for (;;)
    {
        /* Create the string that is sent. */
        sprintf(cString,
                "Standard send message number %lurn",
                ulCount);

        /* Send the string to the UDP socket.  ulFlags is set to 0, so the standard
        semantics are used.  That means the data from cString[] is copied
        into a network buffer inside FreeRTOS_sendto(), and cString[] can be
        reused as soon as FreeRTOS_sendto() has returned. */
        FreeRTOS_sendto(xSocket,
                        cString,
                        strlen(cString),
                        0,
                        &xDestinationAddress,
                        sizeof(xDestinationAddress));

        ulCount++;

        /* Wait until it is time to send again. */
        vTaskDelay(x1000ms);
    }
}