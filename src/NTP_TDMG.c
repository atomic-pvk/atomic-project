#include "NTP_TDMG.h"

#include "NTP_main_utility.h"

// Socket_t xSocket;
// struct freertos_sockaddr xDestinationAddress;

// struct ntp_s s;
// struct ntp_c c;

// void ntp_init(ntp_r *r , ntp_x *x , const char *pcHostNames[], uint32_t *NTP_server_IPs) {
void ntp_init()
{
    // const TickType_t x1000ms = 1000UL / portTICK_PERIOD_MS;
    // const TickType_t x10000ms = 10000UL / portTICK_PERIOD_MS;

    // uint8_t DNSok;

    // for (int i = 0; i < NMAX; i++)
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

    // memset(&s, sizeof(ntp_s), 0);
    // s.leap = NOSYNC;
    // s.stratum = MAXSTRAT;
    // s.poll = MINPOLL;
    // s.precision = PRECISION;
    // s.p = NULL;

    // memset(&c, sizeof(ntp_c), 0);
}

void assoc_table_init(Assoc_table *table, uint32_t *NTP_server_IPs)
{
    table->entries = (Assoc_info *)malloc(NMAX * sizeof(Assoc_info));
    table->size = 0;

    for (int i = 0; i < NMAX; i++)
    {
        FreeRTOS_printf(("\n\ntrying to mobilize with srcaddr %d\n\n", NTP_server_IPs[i]));
        table->entries[i].peer = mobilize(NTP_server_IPs[i], IPADDR, VERSION, MODE, KEYID,
                                          P_FLAGS);
    }
}

int assoc_table_add(Assoc_table *table, uint32_t srcaddr, char hmode, tstamp xmt)
{
    FreeRTOS_printf(("\n\ntrying to modify assoc\n\n"));
    // Check for duplicate entries
    for (int i = 0; i < table->size; i++)
    {
        FreeRTOS_printf(("\n\n numbers we are comparing srcaddr: %d | entry srcaddr:%d\n\n", srcaddr, table->entries[i].srcaddr));

        if (table->entries[i].srcaddr == srcaddr)
        {
            FreeRTOS_printf(("\n\nthis is the hmode we're adding to entry nummer %d: %d\n\n", i, hmode));
            table->entries[table->size].hmode = hmode;
            table->entries[table->size].xmt = xmt;
            return 1; // Entry already exists, do not add
        }
    }
    if (table->size >= NMAX)
    {
        return 0;
    }
    // Add new entry
    FreeRTOS_printf(("\n\n setting values for table srcaddr: %d | hmode:%d\n\n", srcaddr, hmode));
    table->entries[table->size].srcaddr = srcaddr;
    table->entries[table->size].hmode = hmode;
    table->entries[table->size].xmt = xmt;
    table->size++;
    return 1;
}

double sqrt(double number)
{
    double low = 0, high = number;
    double mid, precision = 0.00001;

    if (number < 1)
    {
        high = 1;
    }

    while (high - low > precision)
    {
        mid = low + (high - low) / 2;
        if (mid * mid < number)
        {
            low = mid;
        }
        else
        {
            high = mid;
        }
    }

    return (low + high) / 2;
}