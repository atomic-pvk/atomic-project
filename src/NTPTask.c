#include "NTPTask.h"

#include "NTP_main_utility.h"
#include "NTP_peer.h"

static void vNTPTaskSendUsingStandardInterface(void *pvParameters);

uint32_t NTP1_server_IP;
Socket_t xSocket;
struct freertos_sockaddr xDestinationAddress;

void vStartNTPClientTasks_SingleTasks(uint16_t usTaskStackSize,
                                      UBaseType_t uxTaskPriority)
{
    BaseType_t x;

    /* Create the echo client tasks. */

    xTaskCreate(vNTPTaskSendUsingStandardInterface, /* The function that implements the task. */
                "Echo0",                            /* Just a text name for the task to aid debugging. */
                usTaskStackSize,                    /* The stack size is defined in FreeRTOSIPConfig.h. */
                (void *)x,                          /* The task parameter, not used in this case. */
                uxTaskPriority,                     /* The priority assigned to the task is defined in FreeRTOSConfig.h. */
                NULL);                              /* The task handle is not used. */
}

#define NUM_NTPSERVERS 5

static void vNTPTaskSendUsingStandardInterface(void *pvParameters)
{
    // setup sleep
    const TickType_t x1000ms = 1000UL / portTICK_PERIOD_MS;
    const TickType_t x10000ms = 10000UL / portTICK_PERIOD_MS;

    /* Create the socket. */
    xSocket = FreeRTOS_socket(FREERTOS_AF_INET4,   /* Used for IPv4 UDP socket. */
                                                   /* FREERTOS_AF_INET6 can be used for IPv6 UDP socket. */
                              FREERTOS_SOCK_DGRAM, /*FREERTOS_SOCK_DGRAM for UDP.*/
                              FREERTOS_IPPROTO_UDP);

    /* Check the socket was created. */
    configASSERT(xSocket != FREERTOS_INVALID_SOCKET);

    // Array of hostname strings
    const char *pcHostNames[NUM_NTPSERVERS] = {
        "sth1.ntp.se",
        "sth2.ntp.se",
        "svl1.ntp.se",
        "mmo1.ntp.se",
        "lul1.ntp.se"};

    uint32_t NTP_server_IPs[NUM_NTPSERVERS];
    uint8_t DNSok;

    for (int i = 0; i < NUM_NTPSERVERS; i++)
    {
        DNSok = 0;
        while (DNSok == 0)
        {
            /* get the IP of the NTP server with FreeRTOS_gethostbyname */
            NTP_server_IPs[i] = FreeRTOS_gethostbyname(pcHostNames[i]);

            if (NTP_server_IPs[i] == 0)
            {
                FreeRTOS_printf(("\n\nDNS lookup failed, trying again in 1 second. \n\n"));
                vTaskDelay(x1000ms);
            }
            else
            {
                // print successful and the hostname
                FreeRTOS_printf(("\n\nDNS lookup successful for %s\n\n", pcHostNames[i]));
                DNSok = 1;
            }
        }
    }

    /* Setup destination address */
    xDestinationAddress.sin_family = FREERTOS_AF_INET4;         // or FREERTOS_AF_INET6 if the destination's IP is IPv6.
    xDestinationAddress.sin_address.ulIP_IPv4 = NTP1_server_IP; // destination IP
    xDestinationAddress.sin_port = FreeRTOS_htons(123);         // dest port
    xDestinationAddress.sin_len = (uint8_t)sizeof(struct freertos_sockaddr);

    // struct ntp_p *p; /* peer structure pointer */
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

    // stupid but just test
    free(r);

    uint32_t ulCount = 0UL;
    for (;;)
    {
        for (int i = 0; i < NUM_NTPSERVERS; i++)
        {
            // set the destination IP to the current NTP server
            NTP1_server_IP = NTP_server_IPs[i];
            xDestinationAddress.sin_address.ulIP_IPv4 = NTP1_server_IP;

            // send packet
            xmit_packet(x);
            r = malloc(sizeof(ntp_r));
            r = recv_packet();
            // r->rec = FreeRTOS_ntohl(r->rec);
            // time_t timeInSeconds = (time_t)(r->rec - 2208988800ull);

            // print rec in time, it is 64 bits where 32 first bits are seconds and 32 last bits are fractions of seconds
            time_t timeInSeconds = (time_t)((r->rec >> 32) - 2208988800ull);
            time_t frac = (time_t)(r->rec & 0xFFFFFFFF);
            FreeRTOS_printf(("\n\n Time according to %s: %s.%d\n", pcHostNames[i], ctime(&timeInSeconds), frac));

            TickType_t test = xTaskGetTickCount();
            FreeRTOS_printf(("\n\n TICKS for %s: %d\n", pcHostNames[i], test));

            // free the memory
            free(r);
        }

        FreeRTOS_printf(("\n\nSleeping for 10 seconds before next get\n\n"));

        // sleep for 10 seconds
        vTaskDelay(x10000ms);
    }
}
