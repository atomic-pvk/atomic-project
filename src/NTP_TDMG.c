#include "NTP_TDMG.h"

#include "NTP_main_utility.h"

#include "NTPTask.h"

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
        table->entries[i].peer = mobilize(NTP_server_IPs[i], IPADDR, VERSION, MODE, KEYID,
                                          P_FLAGS);
    }
}

int assoc_table_add(Assoc_table *table, uint32_t srcaddr, char hmode, tstamp xmt)
{
    // Check for duplicate entries
    for (int i = 0; i < table->size; i++)
    {
        if (table->entries[i].srcaddr == srcaddr)
        {
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

uint64_t subtract_uint64_t(uint64_t x, uint64_t y)
{
    uint32_t xFrac = (uint32_t)(x & 0xFFFFFFFF);
    uint32_t yFrac = (uint32_t)(y & 0xFFFFFFFF);
    uint32_t xS = (uint32_t)(x >> 32);
    uint32_t yS = (uint32_t)(y >> 32);

    int borrow = 0;
    if (xFrac < yFrac)
    {
        borrow = 1; // Need to borrow from the high part
    }

    uint32_t low = xFrac - yFrac;
    if (borrow)
    {
        low += 1U << 31; // Correctly adjust for borrow in 32-bit arithmetic
        low += 1U << 31;
    }

    uint32_t high = xS - yS - borrow;

    uint64_t result = ((uint64_t)high << 32) | low; // Assemble the high and low parts back into a 64-bit integer

    return result;
}

uint64_t add_uint64_t(uint64_t x, uint64_t y)
{
    uint32_t xHigh = (uint32_t)(x >> 32);       // High 32 bits of x
    uint32_t xLow = (uint32_t)(x & 0xFFFFFFFF); // Low 32 bits of x
    uint32_t yHigh = (uint32_t)(y >> 32);       // High 32 bits of y
    uint32_t yLow = (uint32_t)(y & 0xFFFFFFFF); // Low 32 bits of y

    // Add the low parts and check for overflow
    uint32_t sumLow = xLow + yLow;
    uint32_t carry = sumLow < xLow ? 1 : 0; // If overflow occurred, set carry

    // Add the high parts and include the carry
    uint32_t sumHigh = xHigh + yHigh + carry;

    // Combine the high and low parts into a 64-bit result
    uint64_t result = ((uint64_t)sumHigh << 32) | sumLow;

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
    uint32_t fractions = (timestamp & 0xFFFFFFFF);

    // double fractionAsSecond = fractions / (double)0xFFFFFFFF;
    double fractionAsSecond = fractions / 4294967296.0; // / 4294967296.0 = 2^32 

    // time_t timeInSeconds = (time_t)((r->rec >> 32) - 2208988800ull);
    // uint32_t frac = (uint32_t)(r->rec & 0xFFFFFFFF);
    // FreeRTOS_printf(("\n\n Time according to %s: %s.%u\n", pcHostNames[i], ctime(&timeInSeconds), frac));

    // Print the timestamp with the comment
    FreeRTOS_printf(("%s Timestamp: %s.%.10d seconds\n", comment, ctime(&seconds), fractionAsSecond));
}

#define MAX_UINT64_DIGITS 20 // Maximum digits in uint64_t

void uint64_to_str(uint64_t value, char *str)
{
    char temp[MAX_UINT64_DIGITS];
    int i = 0;

    // Handle 0 explicitly, otherwise empty string is printed for 0
    if (value == 0)
    {
        str[0] = '0';
        str[1] = '\0';
        return;
    }

    // Process individual digits
    while (value != 0)
    {
        temp[i++] = (value % 10) + '0'; // get next digit
        value = value / 10;             // remove it from the number
    }

    // Reverse the string
    int j = 0;
    while (i > 0)
    {
        str[j++] = temp[--i];
    }
    str[j] = '\0'; // Null-terminate string
}

void FreeRTOS_printf_wrapper(const char *format, uint64_t value)
{
    char buffer[MAX_UINT64_DIGITS + 50]; // Additional space for message text
    char numberStr[MAX_UINT64_DIGITS];
    uint64_to_str(value, numberStr);
    sprintf(buffer, format, numberStr);
    FreeRTOS_printf(("%s\n", buffer));
}

#define MAX_DOUBLE_DIGITS 30 // A buffer size that should handle most cases

void double_to_str(double value, char *str, int precision)
{
    if (precision > 9)
        precision = 9; // Limit precision to 9 digits for simplicity

    // Handle negative numbers
    int index = 0;
    if (value < 0)
    {
        str[index++] = '-';
        value = -value; // Make the value positive for easier processing
    }

    // Extract integer part
    uint64_t int_part = (uint64_t)value;
    double fractional_part = value - (double)int_part;

    // Convert integer part to string
    char int_str[MAX_DOUBLE_DIGITS];
    uint64_to_str(int_part, int_str);
    for (int i = 0; int_str[i] != '\0'; i++)
    {
        str[index++] = int_str[i];
    }

    // Process fractional part if precision is greater than 0
    if (precision > 0)
    {
        str[index++] = '.'; // add decimal point

        // Convert fractional part to a string based on the specified precision
        double multiplier = 1.0;
        for (int i = 0; i < precision; i++)
        {
            multiplier *= 10.0;
        }

        uint64_t fractional_digits = (uint64_t)(fractional_part * multiplier + 0.5); // Round the fractional part

        char frac_str[MAX_DOUBLE_DIGITS];
        uint64_to_str(fractional_digits, frac_str);

        // Add leading zeros if necessary
        int frac_digits_count = 0;
        while (frac_str[frac_digits_count] != '\0')
            frac_digits_count++;

        while (frac_digits_count < precision)
        {
            str[index++] = '0'; // add leading zero
            precision--;
        }

        // Append the rest of the fractional digits
        for (int i = 0; i < frac_digits_count; i++)
        {
            str[index++] = frac_str[i];
        }
    }

    str[index] = '\0'; // Null-terminate the string
}

void FreeRTOS_printf_wrapper_double(const char *format, double value)
{
    char buffer[MAX_DOUBLE_DIGITS + 50]; // Additional space for message text
    double_to_str(value, buffer, 6);     // Example with 6 decimal places
    FreeRTOS_printf(("%s\n", buffer));
}

void print_uint64_as_32_parts(uint64_t number)
{
    uint32_t lower_part = (uint32_t)number;         // Extracts the lower 32 bits
    uint32_t upper_part = (uint32_t)(number >> 32); // Shifts right by 32 bits and extracts the upper 32 bits

    printf("Upper 32 bits: %u\n", upper_part);
    printf("Lower 32 bits: %u\n", lower_part);
}