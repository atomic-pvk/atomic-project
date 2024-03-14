#include "FreeRTOS_IP.h"
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

    /* Send strings to port 10000 on IP address 192.168.0.50. */
    memset( &xDestinationAddress, 0, sizeof( xDestinationAddress ) );
    xDestinationAddress.sin_family = FREERTOS_AF_INET4; //or FREERTOS_AF_INET6 if the destination's IP is IPv6.
    xDestinationAddress.sin_address.ulIP_IPv4 = FreeRTOS_inet_addr("192.168.0.50");
    xDestinationAddress.sin_port = FreeRTOS_htons(10000);
    xDestinationAddress.sin_len = ( uint8_t ) sizeof( xDestinationAddress );

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
