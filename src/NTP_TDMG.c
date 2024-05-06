#include "NTP_TDMG.h"

#include "NTP_main_utility.h"

#include "NTPTask.h"

#include "printf-master/printf.h"

// Socket_t xSocket;
// struct freertos_sockaddr xDestinationAddress;

// struct ntp_s s;
// struct ntp_c c;

void _putchar(char character)
{
  // send char to console etc.
}


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

double subtract_uint64_t(uint64_t x, uint64_t y)
{
    time_t xS = (time_t)(x >> 32);
    uint32_t xFrac = (uint32_t)(x & 0xFFFFFFFF);
    time_t yS = (time_t)(y >> 32);
    uint32_t yFrac = (uint32_t)(y & 0xFFFFFFFF);

    int borrow = 0;
    if (xFrac < yFrac)
    {
        borrow = 1;
    }

    long long high = (long long)(xS - yS - borrow);
    long long low = (long long)(xFrac - yFrac + ((long long)borrow << 32));

    long long result = (high << 32) | (low & 0xFFFFFFFF);

    return (double)result;
}

uint64_t add_uint64_t(uint64_t x, uint64_t y)
{
    time_t xS = (time_t)(x >> 32);
    uint32_t xFrac = (uint32_t)(x & 0xFFFFFFFF);
    time_t yS = (time_t)(y >> 32);
    uint32_t yFrac = (uint32_t)(y & 0xFFFFFFFF);

    uint32_t sumFrac = xFrac + yFrac;
    time_t carry = sumFrac < xFrac ? 1 : 0;

    time_t sumS = xS + yS + carry;

    uint64_t result = ((uint64_t)sumS << 32) | sumFrac;

    return result;
}

void settime(tstamp newTime)
{
    lastTimeStampTstamp = newTime;
    lastTimeStampTick = xTaskGetTickCount();

    // print the tick now
    FreeRTOS_printf(("\n Tick is now: %d\n", lastTimeStampTick));
}

// tstamp gettime()
// {
//     TickType_t currentTick = xTaskGetTickCount();
//     TickType_t tickDifference = currentTick - lastTimeStampTick;

//     uint32_t secondsDifference = tickDifference / 1000;
//     uint32_t fractionsDifference = (tickDifference % 1000) * (0xFFFFFFFF / 1000);

//     // Check if fractionsDifference is larger than one second
//     if (fractionsDifference > 0xFFFFFFFF)
//     {
//         // Increment secondsDifference by 1
//         secondsDifference += 1;
//         // Subtract one second from fractionsDifference
//         fractionsDifference -= 0x100000000;
//     }

//     uint64_t secondsPart = (uint64_t)secondsDifference << 32;
//     uint64_t fractionsPart = fractionsDifference;

//     uint64_t newTime = add_uint64_t(lastTimeStampTstamp, secondsPart);
//     newTime = add_uint64_t(newTime, fractionsPart);

//     return newTime;
// }

// tstamp gettime()
// {
//     TickType_t currentTick = xTaskGetTickCount();
//     TickType_t tickDifference = currentTick - lastTimeStampTick;

//     uint32_t secondsDifference = tickDifference / 1000;
//     uint32_t fractionsDifference = (tickDifference % 1000) * (0xFFFFFFFF / 1000);

//     // Ensure the correct calculation of fractionsDifference
//     fractionsDifference = (uint64_t)(tickDifference % 1000) * (0xFFFFFFFF / 1000);

//     // No need to check if fractionsDifference > 0xFFFFFFFF after fixing scaling
//     // As fractionsDifference is correctly scaled it should not exceed 0xFFFFFFFF
//     // This check and adjustment become unnecessary:
//     // if (fractionsDifference > 0xFFFFFFFF)
//     // {
//     //     secondsDifference += 1;
//     //     fractionsDifference -= 0x100000000;
//     // }

//     uint64_t secondsPart = (uint64_t)secondsDifference << 32;
//     uint64_t fractionsPart = fractionsDifference;

//     uint64_t newTime = add_uint64_t(lastTimeStampTstamp, secondsPart);
//     newTime = add_uint64_t(newTime, fractionsPart);

//     return newTime;
// }

// tstamp gettime()
// {
//     TickType_t currentTick = xTaskGetTickCount();

//     // print the tick now
//     FreeRTOS_printf(("\n Tick is now: %d\n", currentTick));

//     uint64_t tickDifference = currentTick - lastTimeStampTick;
//     lastTimeStampTick = currentTick; // Update last tick

//     uint32_t secondsDifference = tickDifference / 1000;
//     uint64_t fractionsDifference = (tickDifference % 1000) * (0xFFFFFFFF / 1000);

//     // Properly handle the case where fractions roll over one second
//     if (fractionsDifference >= 0x100000000ULL)
//     {
//         secondsDifference += 1;
//         fractionsDifference -= 0x100000000ULL;
//     }

//     uint64_t secondsPart = (uint64_t)secondsDifference << 32;
//     uint64_t fractionsPart = fractionsDifference;

//     uint64_t newTime = add_uint64_t(lastTimeStampTstamp, secondsPart);
//     newTime = add_uint64_t(newTime, fractionsPart);

//     lastTimeStampTstamp = newTime; // Update last timestamp

//     return newTime;
// }

tstamp gettime()
{
    TickType_t currentTick = xTaskGetTickCount();

    // calculate the difference between the current tick and the last tick
    TickType_t tickDifference = currentTick - lastTimeStampTick;

    // calculate tickDifference / 1000 as integer division
    TickType_t numSecondsInTicks = tickDifference / 1000;

    // get the remainder tickDifference % 1000 as the number of milliseconds
    TickType_t numMilliseconds = tickDifference % 1000;

    // Convert milliseconds to fraction part of NTP timestamp
    uint32_t newFractions = (numMilliseconds * 0xFFFFFFFF) / 1000;

    // Extract current fractions and seconds from last timestamp
    uint32_t currentFractions = (uint32_t)(lastTimeStampTstamp & 0xFFFFFFFF);
    uint32_t currentSeconds = (uint32_t)(lastTimeStampTstamp >> 32);

    // Calculate new fraction value and handle overflow
    uint32_t tempFractions = currentFractions + newFractions;
    if (tempFractions < currentFractions)
    {                        // Check for overflow using wrap-around condition
        numSecondsInTicks++; // Increment the seconds part due to overflow
    }

    // Update the seconds part
    tstamp newSeconds = currentSeconds + numSecondsInTicks;

    // Construct the new timestamp without directly manipulating 64-bit values
    // print newSeconds and tempFractions separately (use ctime on newSeconds)
    // FreeRTOS_printf(("\n\n 2 Time according to newSeconds: %s.%.10f\n", ctime(&newSeconds), tempFractions));

    tstamp newTimeStamp = ((tstamp)newSeconds << 32) | (uint32_t)(tempFractions & 0xFFFFFFFF);

    // .536963481 seconds
    // .536963481
    // .536963497
    // .536961241
    // .536961249
    // .3546543389

    return newTimeStamp;
}

void printTimestamp(tstamp timestamp, const char *comment)
{
    // Extract the whole seconds and fractional part from the timestamp
    // uint32_t seconds = (uint32_t)(timestamp >> 32);
    // uint32_t fractions = (uint32_t)(timestamp & 0xFFFFFFFF);
    // double fractionAsSecond = fractions / (double)0xFFFFFFFF;
    time_t seconds = (time_t)((timestamp >> 32) - 2208988800ull);
    uint32_t fractions = (uint32_t)(timestamp & 0xFFFFFFFF);

    // double fractionAsSecond = fractions / (double)0xFFFFFFFF;
    double fractionAsSecond = fractions / 4294967296.0; // / 4294967296.0 = 2^32

    // time_t timeInSeconds = (time_t)((r->rec >> 32) - 2208988800ull);
    // uint32_t frac = (uint32_t)(r->rec & 0xFFFFFFFF);
    // FreeRTOS_printf(("\n\n Time according to %s: %s.%u\n", pcHostNames[i], ctime(&timeInSeconds), frac));

    // Print the timestamp with the comment
    FreeRTOS_printf(("%s Timestamp: %s.%.10f seconds\n", comment, ctime(&seconds), fractionAsSecond));
}
