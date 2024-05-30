#include "NTP_TDMG.h"

#include "NTPTask.h"
#include "NTP_main_utility.h"

/**
 * Initializes the association table with NTP peers.
 *
 * This function allocates memory for the association table and then mobilizes
 * each NTP peer. The IP addresses of the NTP peers are provided in the
 * NTP_server_IPs array.
 *
 * @param NTP_server_IPs An array of IP addresses for the NTP peers.
 */
void assoc_table_init(uint32_t *NTP_server_IPs)
{
    assoc_table->peers = (ntp_p **)malloc(NMAX * sizeof(ntp_p));
    if (assoc_table->peers == NULL)
    {
        // Handle memory allocation failure, e.g. by logging an error message and returning
        FreeRTOS_printf(("Memory allocation failed for association table.\n"));
        return;
    }

    assoc_table->size = NMAX;

    for (int i = 0; i < NMAX; i++)
    {
        assoc_table->peers[i] = mobilize(NTP_server_IPs[i], IPADDR, VERSION, MODE, KEYID, P_FLAGS);
    }
}

/**
 * Deinitializes the association table.
 *
 * This function demobilizes each NTP peer and then frees the memory for the
 * association table.
 */
void assoc_table_deinit()
{
    for (int i = 0; i < assoc_table->size; i++)
    {
        free(assoc_table->peers[i]);
    }

    free(assoc_table->peers);
    assoc_table->peers = NULL;
    assoc_table->size = 0;
}

/**
 * Adds or updates an NTP peer in the association table.
 *
 * @param srcaddr The source address of the NTP peer.
 * @param hmode The host mode of the NTP peer.
 * @param xmt The transmit timestamp of the NTP peer.
 * @return 1 if an existing entry was updated, 0 otherwise.
 */
int assoc_table_add(uint32_t srcaddr, char hmode, tstamp xmt)
{
    // Check for duplicate peers
    for (int i = 0; i < assoc_table->size; i++)
    {
        ntp_p *assoc = assoc_table->peers[i];

        if (assoc->srcaddr == srcaddr && assoc->hmode == hmode)
        {
            assoc->xmt = xmt;
            // assoc->t = c.t;
            return 1;  // Entry already exists
        }
    }
    return 0;
}

/**
 * Updates an NTP peer in the association table.
 *
 * @param p The NTP peer with updated data.
 */
void assoc_table_update(ntp_p *p)
{
    for (int i = 0; i < assoc_table->size; i++)
    {
        ntp_p *assoc = assoc_table->peers[i];
        if (assoc->srcaddr == p->srcaddr && assoc->hmode == p->hmode)
        {
            // Copy the data from p to assoc
            assoc_table->peers[i] = p;
            return;
        }
    }
}

/**
 * Calculates the square root of a number using binary search.
 *
 * @param number The number to calculate the square root of.
 * @return The square root of the number.
 */
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

/**
 * Subtracts two unsigned 64-bit integers and returns the result.
 *
 * @param x The first unsigned 64-bit integer.
 * @param y The second unsigned 64-bit integer.
 * @return The difference between x and y as a signed 64-bit integer.
 */
int64_t subtract_uint64_t(uint64_t x, uint64_t y)
{
    int neg = 0;
    uint32_t xFrac = (uint32_t)(x & 0xFFFFFFFF);
    uint32_t yFrac = (uint32_t)(y & 0xFFFFFFFF);
    uint32_t xS = (uint32_t)(x >> 32);
    uint32_t yS = (uint32_t)(y >> 32);

    if (xS < yS || (xS == yS && xFrac < yFrac))
    {
        neg = 1;
        uint64_t temp = x;
        x = y;
        y = temp;
    }

    int borrow = 0;
    if (xFrac < yFrac)
    {
        borrow = 1;  // Need to borrow from the high part
    }

    uint32_t low = xFrac - yFrac;
    if (borrow)
    {
        low += 1U << 31;  // Correctly adjust for borrow in 32-bit arithmetic
        low += 1U << 31;
    }

    uint32_t high = xS - yS - borrow;

    uint64_t result = ((uint64_t)high << 32) | low;  // Assemble the high and low parts back into a 64-bit integer
    result = (neg) ? -result : result;               // Apply the sign if necessary

    return result;
}

/**
 * Adds two int64_t numbers and returns the result.
 *
 * @param x The first int64_t number.
 * @param y The second int64_t number.
 * @return The sum of x and y as an int64_t number.
 */
int64_t add_int64_t(int64_t x, int64_t y)
{
    int32_t xFrac = (int32_t)(x & 0xFFFFFFFF);
    int32_t yFrac = (int32_t)(y & 0xFFFFFFFF);
    int32_t xS = (int32_t)(x >> 32);
    int32_t yS = (int32_t)(y >> 32);

    if (xFrac + yFrac < xFrac || xFrac + yFrac < yFrac)
    {
        xS++;
    }

    int32_t low = xFrac + yFrac;

    int32_t high = xS + yS;

    int64_t result = ((int64_t)high << 32) | low;  // Assemble the high and low parts back into a 64-bit integer

    return result;
}

/**
 * Sets the current time to a new value.
 *
 *
 * @param newTime The new time value to set.
 */
void settime(tstamp newTime)
{
    c.t++;
    c.localTime = newTime;
    c.lastTimeStampTick = xTaskGetTickCount();
}

/**
 * @brief Get the time from the current tick count.
 *
 * @param override Flag indicating whether to override small differences in tick count.
 */
void gettime(int override)
{
    TickType_t currentTick = xTaskGetTickCount();

    // calculate the difference between the current tick and the last tick
    TickType_t tickDifference = currentTick - c.lastTimeStampTick;

    // FreeRTOS_printf(("Tick diff\n"));
    // FreeRTOS_printf_wrapper_double("", tickDifference);
    if (tickDifference < 10 && !override) return;  // Ignore small differences
    // FreeRTOS_printf(("changing local time...\n"));

    // calculate tickDifference / 1000 as integer division
    TickType_t numSecondsInTicks = tickDifference / 1000;

    // get the remainder tickDifference % 1000 as the number of milliseconds
    TickType_t numMilliseconds = tickDifference % 1000;

    // Convert milliseconds to fraction part of NTP timestamp
    uint32_t newFractions = numMilliseconds * (FRAC / 1000);

    // Extract current fractions and seconds from last timestamp
    uint32_t currentFractions = (uint32_t)(c.localTime & 0xFFFFFFFF);
    uint32_t currentSeconds = (uint32_t)(c.localTime >> 32);

    // Calculate new fraction value and handle overflow
    uint32_t tempFractions = currentFractions + newFractions;
    if (tempFractions < currentFractions)
    {  // Check for overflow using wrap-around condition
        // FreeRTOS_printf(("Overflow detected\n"));
        numSecondsInTicks++;  // Increment the seconds part due to overflow
    }

    // Update the seconds part
    tstamp newSeconds = currentSeconds + numSecondsInTicks;

    // Construct the new timestamp
    tstamp newTimeStamp = ((tstamp)newSeconds << 32) | (uint32_t)(tempFractions & 0xFFFFFFFF);

    c.lastTimeStampTick = currentTick;
    c.localTime = newTimeStamp;
}

/**
 * Prints a timestamp and a comment to the console.
 *
 * @param timestamp The timestamp to print.
 * @param comment The comment to print with the timestamp.
 */
void printTimestamp(tstamp timestamp, const char *comment)
{
    // Extract the whole seconds and fractional part from the timestamp
    time_t seconds = (time_t)((timestamp >> 32) - 2208988800ull);  // Convert NTP epoch to Unix epoch
    uint32_t fractions = (timestamp & 0xFFFFFFFF);

    // Convert fractions to a fraction of a second
    double fractionAsSecond = (double)fractions / FRAC;  // FRAC is presumably defined as 0xFFFFFFFF or 4294967296.0

    // Print the timestamp with the comment
    FreeRTOS_printf(("%s Timestamp: %s. seconds\n", comment, ctime(&seconds)));
    FreeRTOS_printf_wrapper_double("", fractionAsSecond);
}

#define MAX_UINT64_DIGITS 20  // Maximum digits in uint64_t

/**
 * Converts a 64-bit unsigned integer to a string.
 *
 * @param value The number to convert.
 * @param str The output string. This must be large enough to hold all digits of the number plus a null terminator.
 */
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
        temp[i++] = (value % 10) + '0';  // get next digit
        value = value / 10;              // remove it from the number
    }

    // Reverse the string
    int j = 0;
    while (i > 0)
    {
        str[j++] = temp[--i];
    }
    str[j] = '\0';  // Null-terminate string
}

/**
 * A wrapper around the FreeRTOS_printf function that allows printing a 64-bit unsigned integer.
 *
 * @param format The format string for the message. This should contain one %s specifier where the number will be
 * inserted.
 * @param value The number to print.
 */
void FreeRTOS_printf_wrapper(const char *format, uint64_t value)
{
    char numberStr[MAX_UINT64_DIGITS];
    uint64_to_str(value, numberStr);       // Convert the number to a string
    FreeRTOS_printf((format, numberStr));  // Print the formatted message
}

#define MAX_DOUBLE_DIGITS 30  // A buffer size that should handle most cases

/**
 * Converts a double value to a string with a specified precision.
 *
 * @param value The double value to convert.
 * @param str The output string. This must be large enough to hold all digits of the number plus a null terminator.
 * @param precision The number of digits after the decimal point.
 */
void double_to_str(double value, char *str, int precision)
{
    if (precision > 9) precision = 9;  // Limit precision to 9 digits for simplicity

    // Handle negative numbers
    int index = 0;
    if (value < 0)
    {
        str[index++] = '-';
        value = -value;  // Make the value positive for easier processing
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
        str[index++] = '.';  // add decimal point

        // Convert fractional part to a string based on the specified precision
        double multiplier = 1.0;
        for (int i = 0; i < precision; i++)
        {
            multiplier *= 10.0;
        }

        uint64_t fractional_digits = (uint64_t)(fractional_part * multiplier + 0.5);  // Round the fractional part

        char frac_str[MAX_DOUBLE_DIGITS];
        uint64_to_str(fractional_digits, frac_str);

        // Add leading zeros if necessary
        int frac_digits_count = 0;
        while (frac_str[frac_digits_count] != '\0') frac_digits_count++;

        while (frac_digits_count < precision)
        {
            str[index++] = '0';  // add leading zero
            precision--;
        }

        // Append the rest of the fractional digits
        for (int i = 0; i < frac_digits_count; i++)
        {
            str[index++] = frac_str[i];
        }
    }

    str[index] = '\0';  // Null-terminate the string
}

/**
 * A wrapper around the FreeRTOS_printf function that allows printing a double value.
 *
 * @param format The format string for the message. This should contain one %s specifier where the number will be
 * inserted.
 * @param value The double value to print.
 */
void FreeRTOS_printf_wrapper_double(const char *format, double value)
{
    char buffer[MAX_DOUBLE_DIGITS + 50];  // Additional space for message text
    double_to_str(value, buffer, 10);     // Example with 6 decimal places
    FreeRTOS_printf(("%s\n", buffer));
}

/**
 * Prints a 64-bit unsigned integer as two 32-bit parts.
 *
 * @param number The 64-bit unsigned integer to print.
 */
void print_uint64_as_32_parts(uint64_t number)
{
    uint32_t lower_part = (uint32_t)number;          // Extracts the lower 32 bits
    uint32_t upper_part = (uint32_t)(number >> 32);  // Shifts right by 32 bits and extracts the upper 32 bits

    printf("Upper 32 bits: %u\n", upper_part);
    printf("Lower 32 bits: %u\n", lower_part);
}