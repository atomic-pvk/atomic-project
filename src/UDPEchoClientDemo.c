#include "FreeRTOS_IP.h"
#include "UDPEchoClientDemo.h"

#include "NTP_main_utility.h"
#include "NTP_peer.h"
#include "FreeRTOS_Sockets.h"

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
    Socket_t xSocket;
    struct freertos_sockaddr xDestinationAddress;
    uint8_t cString[50];
    uint32_t ulCount = 0UL;
    const TickType_t x1000ms = 1000UL / portTICK_PERIOD_MS;

    struct ntp_r *r;
    r = malloc(sizeof(ntp_r));
    struct ntp_x *x;
    x = malloc(sizeof(ntp_x));
    struct ntp_s s;
    struct ntp_c c;

    memset(r, 0, sizeof(ntp_r));
    r->dstaddr = DSTADDR;
    r->version = VERSION;
    r->leap = NOSYNC;
    r->mode = MODE;
    r->stratum = MAXSTRAT;
    r->poll = MINPOLL;
    r->precision = PRECISION;

    memset(x, 0, sizeof(ntp_x));
    x->dstaddr = DSTADDR;
    x->version = VERSION;
    x->leap = NOSYNC;
    x->mode = MODE;
    x->stratum = MAXSTRAT;
    x->poll = MINPOLL;
    x->precision = PRECISION;

    memset(&s, sizeof(ntp_s), 0);
    s.leap = NOSYNC;
    s.stratum = MAXSTRAT;
    s.poll = MINPOLL;
    s.precision = PRECISION;
    s.p = NULL;

    memset(&c, sizeof(ntp_c), 0);

    // xmit_packet(x);

    /* Send strings to port 9000 on IP address 192.168.69.2 (replace with suitable IP at run time) */
    /* use https://github.com/FreeRTOS/FreeRTOS-Libraries-Integration-Tests/tree/main/tools/echo_server for udp echo server */
    memset(&xDestinationAddress, 0, sizeof(xDestinationAddress));
    xDestinationAddress.sin_family = FREERTOS_AF_INET4; // or FREERTOS_AF_INET6 if the destination's IP is IPv6.
    xDestinationAddress.sin_address.ulIP_IPv4 = FreeRTOS_inet_addr("194.58.202.20");
    xDestinationAddress.sin_port = FreeRTOS_htons(123); // outgoing port
    xDestinationAddress.sin_len = (uint8_t)sizeof(xDestinationAddress);

    ntp_packet *pkt = malloc(sizeof(ntp_packet)); // Allocate memory for the packet

    memset(pkt, 0, sizeof(ntp_packet)); // Clear the packet struct

    // Setting li_vn_mode combining leap, version, and mode
    pkt->li_vn_mode = (x->leap & 0x03) << 6 | (x->version & 0x07) << 3 | (x->mode & 0x07);

    // Directly mapping other straightforward fields
    pkt->stratum = x->stratum;
    pkt->poll = x->poll;
    pkt->precision = x->precision;

    // Assuming simple direct assignments for demonstration purposes
    // Transformations might be necessary depending on actual data types and requirements
    pkt->refId = x->srcaddr; // Just an example mapping

    // Serialize the packet to be ready for sending
    unsigned char buffer[sizeof(ntp_packet)];
    memcpy(buffer, pkt, sizeof(ntp_packet)); // Here the packet is fully prepared and can be sent

    /* Create the socket. */
    xSocket = FreeRTOS_socket(FREERTOS_AF_INET4,   /* Used for IPv4 UDP socket. */
                                                   /* FREERTOS_AF_INET6 can be used for IPv6 UDP socket. */
                              FREERTOS_SOCK_DGRAM, /*FREERTOS_SOCK_DGRAM for UDP.*/
                              FREERTOS_IPPROTO_UDP);

    /* Check the socket was created. */
    configASSERT(xSocket != FREERTOS_INVALID_SOCKET);

    /* NOTE: FreeRTOS_bind() is not called.  This will only work if
    ipconfigALLOW_SOCKET_SEND_WITHOUT_BIND is set to 1 in FreeRTOSIPConfig.h. */

    int one = 1;
    for (;;)
    {
        /* Create the string that is sent. */
        sprintf(cString,
                "Standard send message number %lurn",
                ulCount);

        int32_t iReturned1;

        /* Send the string to the UDP socket.  ulFlags is set to 0, so the standard
        semantics are used.  That means the data from cString[] is copied
        into a network buffer inside FreeRTOS_sendto(), and cString[] can be
        reused as soon as FreeRTOS_sendto() has returned. */
        iReturned1 = FreeRTOS_sendto(xSocket,
                                     buffer,
                                     sizeof(buffer),
                                     0,
                                     &xDestinationAddress,
                                     sizeof(xDestinationAddress));

        if (iReturned1 == sizeof(buffer))
        {
            FreeRTOS_printf(("\n\nSent packet\n\n"));
        }
        else
        {
            FreeRTOS_printf(("\nFailed to send packet\n"));
        }

        ulCount++;

        /* Wait until it is time to send again. */
        //    vTaskDelay(x1000ms);

        ntp_packet *pkt = malloc(sizeof(ntp_packet)); // Allocate memory for the packet
        memset(pkt, 0, sizeof(ntp_packet));           // Clear the packet struct
        unsigned char bufferRecv[sizeof(ntp_packet)];
        int32_t iReturned;
        struct freertos_sockaddr xSourceAddress;
        socklen_t xAddressLength = sizeof(xSourceAddress);

        iReturned = FreeRTOS_recvfrom(xSocket,
                                      bufferRecv,
                                      sizeof(ntp_packet),
                                      0,
                                      &xSourceAddress,
                                      &xAddressLength);

        if (iReturned == sizeof(ntp_packet))
        {
            FreeRTOS_printf(("Received packet size is correct\n"));
        }
        else
        {
            FreeRTOS_printf(("\n\n\n\n\n\nFailed to receive full packet, received size: %d\n", iReturned));
        }

        if (iReturned == sizeof(ntp_packet))
        {
            FreeRTOS_printf(("Received packet size is correct\n"));

            // Convert the timestamp from network byte order to host byte order
            // Assuming bufferRecv points to an unsigned char array containing the data.

            uint32_t time = FreeRTOS_ntohl(
                (bufferRecv[35] << 24) |
                (bufferRecv[34] << 16) |
                (bufferRecv[33] << 8) |
                bufferRecv[32]);

            // print the time integer
            FreeRTOS_printf(("Time in integer format only: %lu\n", time));

            // uint32_t time = FreeRTOS_ntohl(((*(uint32_t*)bufferRecv) >> 12) & 0xFFFF);

            // uint32_t time = FreeRTOS_ntohl(*((uint32_t *)(bufferRecv + 12)));
            // one++;

            // FreeRTOS_printf(("time thing: %lu\n", time));

            // Convert NTP time (seconds since 1900) to UNIX time (seconds since 1970)
            time_t timeInSeconds = (time_t)(time)-2208988800ull;

            FreeRTOS_printf(("Time when converted using ctime: %s\n", ctime(&timeInSeconds)));
        }
        else
        {
            FreeRTOS_printf(("Failed to receive full packet, received size: %d\n", iReturned));
        }

        vTaskDelay(x1000ms);
    }
}
