#include "NTPTask.h"

#include "NTP_main_utility.h"
#include "NTP_peer.h"
#include "NTP_poll.h"
#include "NTP_vfo.h"

SemaphoreHandle_t timeMutex;

// Global NTP client variables
uint32_t NTP1_server_IP;
Socket_t xSocket;
struct freertos_sockaddr xDestinationAddress;
Assoc_table *assoc_table;
struct ntp_s s;
struct ntp_c c;

// Task function declaration
static void vNTPTask(void *pvParameters);
static void vNTPTaskInterrupt(void *pvParameters);

// Function to start NTP client task
void vStartNTPClientTasks_SingleTasks(uint16_t usTaskStackSize, UBaseType_t uxTaskPriority)
{
    BaseType_t x;

    FreeRTOS_printf(("Starting NTPTask. \n"));
    // Create the NTP client task
    xTaskCreate(vNTPTask,        /* Task function */
                "NTPSyncTask",   /* Task name */
                usTaskStackSize, /* Stack size */
                (void *)x,       /* Task parameter (unused) */
                uxTaskPriority,  /* Task priority */
                NULL);           /* No task handle required */
    // Create the NTP client task for clock adjustment
    // xTaskCreate(vNTPTaskInterrupt,    /* Task function */
    //             "NTPClockAdjustTask", /* Task name */
    //             usTaskStackSize / 25, /* Stack size */
    //             (void *)x,            /* Task parameter (unused) */
    //             uxTaskPriority,       /* Task priority */
    //             NULL);                /* No task handle required */
}

void initTimeMutex() { timeMutex = xSemaphoreCreateMutex(); }

static void init_network_and_DNS(const char *pcHostNames[], uint32_t NTP_server_IPs[])
{
    const TickType_t x1000ms = 1000UL / portTICK_PERIOD_MS;
    xSocket = FreeRTOS_socket(FREERTOS_AF_INET4,   /* IPv4 UDP socket */
                              FREERTOS_SOCK_DGRAM, /* Datagram (UDP) */
                              FREERTOS_IPPROTO_UDP);

    // Check socket creation
    configASSERT(xSocket != FREERTOS_INVALID_SOCKET);

    // Resolve hostnames to IPs
    for (int i = 0; i < NMAX; i++)
    {
        for (int retry = 0; retry < 10; retry++)
        {
            // Resolve hostname
            NTP_server_IPs[i] = FreeRTOS_gethostbyname(pcHostNames[i]);

            if (NTP_server_IPs[i] == 0)
            {
                FreeRTOS_printf(("DNS lookup failed, retrying in 1 second. \n"));
                vTaskDelay(x1000ms);  // Retry delay
            }
            else
            {
                FreeRTOS_printf(("DNS lookup successful for %s\n\n", pcHostNames[i]));
                break;
            }

            // Exit if max retries reached
            if (retry == 9)
            {
                FreeRTOS_printf(("DNS lookup failed after multiple attempts. Exiting task.\n"));
                return;
            }
        }
    }

    // Set up destination address
    xDestinationAddress.sin_family = FREERTOS_AF_INET4;
    xDestinationAddress.sin_port = FreeRTOS_htons(123);  // NTP default port
    xDestinationAddress.sin_len = sizeof(struct freertos_sockaddr);
}

// Task function for NTP client
static void vNTPTask(void *pvParameters)
{
    // Sleep intervals in milliseconds
    const TickType_t x100ms = 100UL / portTICK_PERIOD_MS;
    const TickType_t x1000ms = 1000UL / portTICK_PERIOD_MS;
    const TickType_t x10000ms = 10000UL / portTICK_PERIOD_MS;

    // test demo server ntp-pvk-atomic.hejduk.se
    const char *pcHostNames[NMAX] = {"sth1.ntp.se", "sth2.ntp.se", "svl1.ntp.se", "mmo1.ntp.se", "lul1.ntp.se"};

    // Array to store resolved IP addresses of NTP servers
    uint32_t NTP_server_IPs[NMAX] = {0};
    FreeRTOS_printf(("Initializing network and dns \n"));
    init_network_and_DNS(pcHostNames, NTP_server_IPs);
    FreeRTOS_printf(("Initializing mutex \n"));
    initTimeMutex();

    FreeRTOS_printf(("Initializing datastructures \n"));
    // Allocate and initialize NTP packet structures
    struct ntp_r *r = malloc(sizeof(ntp_r));
    struct ntp_x *x = calloc(1, sizeof(ntp_x));  // Zero-initialized

    x->version = VERSION;
    x->mode = MODE;
    x->stratum = MAXSTRAT;
    x->poll = MINPOLL;
    x->precision = -18;

    // Initialize NTP system and control structures
    memset(&s, 0, sizeof(ntp_s));
    s.leap = NOSYNC;
    s.stratum = MAXSTRAT;
    s.poll = MINPOLL;
    s.precision = -18;

    memset(&c, 0, sizeof(ntp_c));
    c.lastTimeStampTick = 0;

    // Handle frequency settings
    if (FREQ_EXISTS)
    {
        c.freq = (double)configTICK_RATE_HZ;
        rstclock(FSET, 0, 0);
    }
    else
    {
        rstclock(NSET, 0, 0);
    }

    c.jitter = LOG2D(s.precision);
    FreeRTOS_printf(("Initializing association table \n"));
    assoc_table_init(NTP_server_IPs);

    // Initialize time with the first NTP server
    FreeRTOS_printf(("Initializing time with the first NTP server \n"));
    NTP1_server_IP = NTP_server_IPs[0];
    xDestinationAddress.sin_address.ulIP_IPv4 = NTP1_server_IP;
    x->srcaddr = NTP1_server_IP;

    prep_xmit(x);    // Prepare for transmission
    xmit_packet(x);  // Send initial request
    recv_packet(r);  // Receive the response

    x->xmt = r->xmt;  // Set initial transmission timestamp
    settime(x->xmt);
    gettime(1);

    FreeRTOS_printf(("Starting synchronization loop\n"));
    for (int i = 0; i < NMAX; i++)
    {
        // Update destination address with current NTP server
        NTP1_server_IP = NTP_server_IPs[i];

        FreeRTOS_printf(("Getting time from IP (%s): %lu.%lu.%lu.%lu\n\n", pcHostNames[i], (NTP1_server_IP & 0xFF),
                         ((NTP1_server_IP >> 8) & 0xFF), ((NTP1_server_IP >> 16) & 0xFF),
                         ((NTP1_server_IP >> 24) & 0xFF)));

        prep_xmit(x);    // Prepare NTP packet
        xmit_packet(x);  // Send request
        recv_packet(r);  // Receive response
        FreeRTOS_printf(("Start the algorithms\n"));

        // Process the received response
        receive(r);
        vTaskDelay(x1000ms);  // Delay before next init
    }
    FreeRTOS_printf(("Synchronization loop finished\n"));

    // return;

    // Main NTP client loop
    FreeRTOS_printf(("Start polling loop\n"));
    for (;;)
    {
        // gettime(1);  // Get current time

        clock_adjust(r);  // Adjust the clock

        vTaskDelay(x100ms);  // Delay before next adjustment

        // recv_packet(r);      // Receive response
        // receive(r);
    }

    // Cleanup allocated memory (typically not reached in FreeRTOS tasks)
    free(r);
    free(x);
    assoc_table_deinit();
}

// static void vNTPTaskInterrupt(void *pvParameters)
// {
//     // Sleep intervals in milliseconds
//     const TickType_t x100ms = 100UL / portTICK_PERIOD_MS;
//     const TickType_t x1000ms = 1000UL / portTICK_PERIOD_MS;
//     const TickType_t x10000ms = 10000UL / portTICK_PERIOD_MS;

//     while (!init)
//     {
//         vTaskDelay(x1000ms);
//     }
//     for (;;)
//     {
//         FreeRTOS_printf(("Adjust clock\n"));
//         clock_adjust();  // Adjust the clock
//         FreeRTOS_printf(("Adjusted clock\n"));

//         vTaskDelay(x100ms);  // Delay before next adjustment
//     }
// }