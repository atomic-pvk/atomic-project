#include "NTP_TDMG.h"

// Socket_t xSocket;
// struct freertos_sockaddr xDestinationAddress;

struct ntp_s s;
struct ntp_c c;
struct ntp_p p;

// void ntp_init(ntp_r *r , ntp_x *x , const char *pcHostNames[], uint32_t *NTP_server_IPs) {
void ntp_init() {
    // const TickType_t x1000ms = 1000UL / portTICK_PERIOD_MS;
    // const TickType_t x10000ms = 10000UL / portTICK_PERIOD_MS;

    // uint8_t DNSok;

    // for (int i = 0; i < NUM_NTPSERVERS; i++)
    // {
    //     DNSok = 0;
    //     while (DNSok == 0)
    //     {
    //         /* get the IP of the NTP server with FreeRTOS_gethostbyname */
    //         NTP_server_IPs[i] = FreeRTOS_gethostbyname(pcHostNames[i]);

    //         if (NTP_server_IPs[i] == 0)
    //         {
    //             FreeRTOS_printf(("\n\nDNS lookup failed, trying again in 1 second. \n\n"));
    //             vTaskDelay(x1000ms);
    //         }
    //         else
    //         {
    //             // print successful and the hostname
    //             FreeRTOS_printf(("\n\nDNS lookup successful for %s\n\n", pcHostNames[i]));
    //             DNSok = 1;
    //         }
    //     }
    // }

    // // struct ntp_p *p; /* peer structure pointer */

    // memset(r, 0, sizeof(ntp_r));
    // r->dstaddr = DSTADDR;
    // r->version = VERSION;
    // r->leap = NOSYNC;
    // r->mode = MODE;
    // r->stratum = MAXSTRAT;
    // r->poll = MINPOLL;
    // r->precision = PRECISION;

    // memset(x, 0, sizeof(ntp_x));
    // x->dstaddr = DSTADDR;
    // x->version = VERSION;
    // x->leap = NOSYNC;
    // x->mode = MODE;
    // x->stratum = MAXSTRAT;
    // x->poll = MINPOLL;
    // x->precision = PRECISION;

    memset(&p, sizeof(ntp_p), 0);

    memset(&s, sizeof(ntp_s), 0);
    s.leap = NOSYNC;
    s.stratum = MAXSTRAT;
    s.poll = MINPOLL;
    s.precision = PRECISION;
    s.p = NULL;

    memset(&c, sizeof(ntp_c), 0);
}