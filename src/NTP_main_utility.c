#include "NTP_main_utility.h"

#include "NTPTask.h"
#include "NTP_peer.h"

/**
 * This function creates a new NTP peer structure and initializes it.
 *
 * @param srcaddr - The IP source address.
 * @param dstaddr - The IP destination address.
 * @param version - The NTP version.
 * @param mode - The host mode.
 * @param keyid - The key identifier.
 * @param flags - The peer flags.
 *
 * @return A pointer to the new NTP peer structure, or NULL if the memory allocation failed.
 */
struct ntp_p *mobilize(uint32_t srcaddr, /* IP source address, change from ipaddr to uint32_t */
                       uint32_t dstaddr, /* IP destination address, change from ipaddr to uint32_t */
                       int version,      /* version */
                       int mode,         /* host mode */
                       int keyid,        /* key identifier */
                       int flags         /* peer flags */
)
{
    struct ntp_p *p = calloc(1, sizeof(struct ntp_p)); /* allocate and zero-initialize association memory */

    if (p == NULL)
    {
        return NULL; /* return NULL if memory allocation failed */
    }

    p->srcaddr = srcaddr;
    p->dstaddr = dstaddr;
    p->version = version;
    p->hmode = mode;
    p->keyid = keyid;
    p->hpoll = MINPOLL;
    clear(p, X_INIT);
    p->flags = flags;

    return p;
}

/**
 * This function searches for a matching association in the association table.
 *
 * @param r - A pointer to the received NTP packet.
 *
 * @return A pointer to the matching association, or NULL if no match is found.
 */
struct
    ntp_p /* peer structure pointer or NULL */
        *
        find_assoc(struct ntp_r *r /* receive packet pointer */
        )
{
    for (int i = 0; i < assoc_table->size; i++)
    {
        ntp_p *assoc = assoc_table->peers[i];
        if (r->srcaddr == assoc->srcaddr)
        {
            return (assoc);
        }
    }
    return (NULL);
}

/*
 * md5() - compute message digest
 */
digest md5(int keyid /* key identifier */
)
{
    /*
     * Compute a keyed cryptographic message digest.  The key
     * identifier is associated with a key in the local key cache.
     * The key is prepended to the packet header and extension fields
     * and the result hashed by the MD5 algorithm as described in
     * RFC 1321.  Return a MAC consisting of the 32-bit key ID
     * concatenated with the 128-bit digest.
     */
    return (/* MD5 digest */ 0);
}

/**
 * This function swaps the byte order of the fields in an NTP packet.
 *
 * @param pkt - A pointer to the NTP packet.
 *
 * The function uses the FreeRTOS_htonl function to perform the byte order swap. This function converts a 32-bit number
 * from host byte order to network byte order. Network byte order is big-endian, so this function is necessary when
 * sending data over the network on a little-endian system.
 */
static void prv_swap_fields(struct ntp_packet *pkt)
{
    /* NTP messages are big-endian */
    pkt->rootDelay = FreeRTOS_htonl(pkt->rootDelay);
    pkt->rootDispersion = FreeRTOS_htonl(pkt->rootDispersion);

    pkt->refTm_s = FreeRTOS_htonl(pkt->refTm_s);
    pkt->refTm_f = FreeRTOS_htonl(pkt->refTm_f);

    pkt->origTm_s = FreeRTOS_htonl(pkt->origTm_s);
    pkt->origTm_f = FreeRTOS_htonl(pkt->origTm_f);

    pkt->rxTm_s = FreeRTOS_htonl(pkt->rxTm_s);
    pkt->rxTm_f = FreeRTOS_htonl(pkt->rxTm_f);

    pkt->txTm_s = FreeRTOS_htonl(pkt->txTm_s);
    pkt->txTm_f = FreeRTOS_htonl(pkt->txTm_f);
}

/**
 * This function creates and initializes an NTP receive structure from an NTP packet.
 *
 * @param r - A pointer to the NTP receive structure to be initialized.
 * @param pkt - A pointer to the NTP packet.
 */
static void prv_create_ntp_r(struct ntp_r *r, ntp_packet *pkt)
{
    r->srcaddr = NTP1_server_IP;  // Set to zero if not known
    r->dstaddr = DSTADDR;         // Set to zero if not known

    // Extract version, leap, and mode from li_vn_mode
    r->leap = (pkt->li_vn_mode >> 6) & 0x3;     // Extract bits 0-1. li, Leap indicator.
    r->version = (pkt->li_vn_mode >> 3) & 0x7;  // Extract bits 2-4. vn, Version number of the protocol.
    r->mode = (pkt->li_vn_mode) & 0x7;          // Extract bits 5-7. mode, Client will pick mode 3 for client

    r->stratum = ((char)pkt->stratum);
    r->poll = (char)pkt->poll;
    r->precision = (s_char)pkt->precision;

    r->rootdelay = pkt->rootDelay;
    r->rootdisp = pkt->rootDispersion;
    r->refid = pkt->refId;

    // Combine seconds and fractions into a single 64-bit NTP timestamp
    r->org = ((tstamp)pkt->origTm_s << 32) | (uint32_t)(pkt->origTm_f & 0xFFFFFFFF);
    r->reftime = ((tstamp)pkt->refTm_s << 32) | (uint32_t)(pkt->refTm_f & 0xFFFFFFFF);
    r->xmt = ((tstamp)pkt->txTm_s << 32) | (uint32_t)(pkt->txTm_f & 0xFFFFFFFF);
    r->rec = ((tstamp)pkt->rxTm_s << 32) | (uint32_t)(pkt->rxTm_f & 0xFFFFFFFF);

    // Set crypto fields to 0 or default values
    r->keyid = 0;
    r->mac = 0;  // Zero out the MAC digest

    gettime(0);
    tstamp dst = c.localTime;
    r->dst = dst;  // Set the timestamp to the ms passed since vTaskStartScheduler started
}

/**
 * This function receives an NTP packet, converts it to an NTP receive structure, and prints the original timestamp.
 *
 * @param r - A pointer to the NTP receive structure to be filled.
 */
void recv_packet(ntp_r *r)
{
    ntp_packet pkt;
    int32_t iReturned;

    struct freertos_sockaddr xSourceAddress;
    socklen_t xAddressLength = sizeof(xSourceAddress);

    memset(r, 0, sizeof(ntp_r));  // Clear the receive packet struct

    FreeRTOS_printf(("Receiving packet...\n"));

    iReturned = FreeRTOS_recvfrom(xSocket, &pkt, sizeof(ntp_packet), 0, &xSourceAddress, &xAddressLength);

    if (iReturned > 0)
    {
        prv_swap_fields(&pkt);

        FreeRTOS_printf(("Packet received\n"));

        // do conversion
        // ntp_packet -> ntp_r
        prv_create_ntp_r(r, &pkt);
        c.localTime = r->rec;
    }
    else
    {
        FreeRTOS_printf(("Error receiving packet\n"));
    }
}

/**
 * This function prepares an NTP transmit structure for sending.
 *
 * @param x - A pointer to the NTP transmit structure to be prepared.
 */
void prep_xmit(ntp_x *x)
{
    FreeRTOS_printf(("Preparing packet for send \n"));
    x->leap = s.leap;
    x->stratum = s.stratum;

    // Root delay and dispersion can come from a server or client
    x->rootdelay = D2FP(s.rootdelay);
    x->rootdisp = D2FP(s.rootdisp);
    x->refid = s.refid;

    // Assuming tstamp is a type that can be split into seconds and fraction of a second
    x->reftime = s.reftime;
    x->org = x->xmt;
    x->rec = c.localTime;
    gettime(1);
    x->xmt = c.localTime;
}

/**
 * Translates the fields of an ntp_x structure to an ntp_packet structure.
 *
 * @param x The ntp_x structure containing the fields to be translated.
 * @param pkt The ntp_packet structure to store the translated fields.
 */
static void prv_translate_ntp_x_to_ntp_packet(const ntp_x *x, ntp_packet *pkt)
{
    // Combine leap, version, and mode into li_vn_mode
    pkt->li_vn_mode = (x->leap & 0x03) << 6 | (x->version & 0x07) << 3 | (x->mode & 0x07);

    // Set stratum, poll, precision
    pkt->stratum = x->stratum;
    pkt->poll = x->poll;
    pkt->precision = x->precision;

    // Root delay and dispersion can come from a server or client
    pkt->rootDelay = x->rootdelay;
    pkt->rootDispersion = x->rootdisp;
    pkt->refId = x->refid;

    // Assuming tstamp is a type that can be split into seconds and fraction of a second
    pkt->refTm_s = (uint32_t)(x->reftime >> 32);
    pkt->refTm_f = (uint32_t)(x->reftime & 0xFFFFFFFF);
    pkt->origTm_s = (uint32_t)(x->org >> 32);
    pkt->origTm_f = (uint32_t)(x->org & 0xFFFFFFFF);
    pkt->rxTm_s = (uint32_t)(x->rec >> 32);
    pkt->rxTm_f = (uint32_t)(x->rec & 0xFFFFFFFF);
    pkt->txTm_s = (uint32_t)(x->xmt >> 32);
    pkt->txTm_f = (uint32_t)(x->xmt & 0xFFFFFFFF);
}

/**
 * @brief Transmits an NTP packet.
 *
 * This function transmits an NTP packet using the provided transmit packet pointer.
 *
 * @param x Pointer to the transmit packet.
 */
void xmit_packet(ntp_x *x /* transmit packet pointer */
)
{
    /* setup ntp_packet *pkt */
    ntp_packet pkt;  // Allocate memory for the packet

    xDestinationAddress.sin_address.ulIP_IPv4 = NTP1_server_IP;
    x->srcaddr = NTP1_server_IP;

    // set xmit time to current time
    gettime(0);
    x->xmt = c.localTime;

    prv_translate_ntp_x_to_ntp_packet(x, &pkt);
    prv_swap_fields(&pkt);

    FreeRTOS_printf(("Sending packet...\n"));

    // Add the association to the table
    if (!assoc_table_add(x->srcaddr, x->mode, x->xmt))
    {
        FreeRTOS_printf(("Error adding association to table\n"));
    }

    int32_t iReturned;
    socklen_t xAddressLength = sizeof(xDestinationAddress);

    iReturned = FreeRTOS_sendto(xSocket, &pkt, sizeof(ntp_packet), 0, &xDestinationAddress, xAddressLength);

    if (iReturned == sizeof(ntp_packet))
    {
        FreeRTOS_printf(("Sent packet\n"));
    }
    else
    {
        FreeRTOS_printf(("Failed to send packet\n"));
    }
}

/*
 * System clock utility functions
 *
 * There are three time formats: native (Unix), NTP, and floating
 * double.  The get_time() routine returns the time in NTP long format.
 * The Unix routines expect arguments as a structure of two signed
 * 32-bit words in seconds and microseconds (timeval) or nanoseconds
 * (timespec).  The step_time() and adjust_time() routines expect signed
 * arguments in floating double.  The simplified code shown here is for
 * illustration only and has not been verified.
 */
#define JAN_1970 2208988800UL /* 1970 - 1900 in seconds */

/**
 * @brief Updates the local time based on a specified offset in seconds.
 *
 * @param offset_seconds Offset in seconds to be applied to the local time.
 */
static void prv_update_localTime(double offset_seconds, int isAdjustment /* 0 for step, 1 for adjust */)
{
    if (xSemaphoreTake(timeMutex, portMAX_DELAY))
    {
        // printf("Updating local time\n");
        // TickType_t currentTick = xTaskGetTickCount();
        // FreeRTOS_printf_wrapper_double("Offset in ticks: ", currentTick);
        // TickType_t offsetTicks = (TickType_t)(offset_seconds * configTICK_RATE_HZ);
        // FreeRTOS_printf_wrapper_double("Offset in ticks: ", D2LFP(offset_seconds));
        // FreeRTOS_printf_wrapper_double("Offset in ticks: ", offsetTicks);

        // // Calculate the number of seconds in the tick difference
        // TickType_t numSecondsInTicks = offsetTicks / configTICK_RATE_HZ;

        // // Calculate the remaining fraction of ticks converted to NTP fractions
        // TickType_t remainingTicks = offsetTicks % configTICK_RATE_HZ;
        // uint32_t newFractions = (uint32_t)(((double)remainingTicks / configTICK_RATE_HZ) * FRAC);
        // FreeRTOS_printf_wrapper_double("Offset in ticks: ", newFractions);

        // Extract current fractions and seconds from the last known local time
        uint32_t currentFractions = (uint32_t)(c.localTime & 0xFFFFFFFF);
        uint32_t newFractions = (uint32_t)(D2LFP(offset_seconds));
        uint32_t currentSeconds = (uint32_t)(c.localTime >> 32);

        uint32_t tempFractions;
        if (isAdjustment)
        {
            tempFractions = currentFractions + newFractions;
            if (tempFractions < currentFractions)
            {                      // Handle overflow
                currentSeconds++;  // Increment the seconds part due to overflow
            }
        }
        else
        {
            tempFractions = currentFractions - newFractions;
            if (tempFractions > currentFractions)
            {                      // Handle underflow
                currentSeconds--;  // Decrement the seconds part due to underflow
            }
        }
        // FreeRTOS_printf_wrapper_double("", currentFractions);
        // FreeRTOS_printf_wrapper_double("", newFractions);

        // Construct the new timestamp
        uint64_t newTimeStamp = ((uint64_t)currentSeconds << 32) | (tempFractions & 0xFFFFFFFF);

        // Update the control structure with the new tick and local time
        c.localTime = newTimeStamp;

        // Optionally print the updated time for debugging
        // printTimestamp(c.localTime, "Updated time:");

        xSemaphoreGive(timeMutex);
    }
}

/**
 * @brief Steps the time by applying the given clock offset.
 *
 * This function updates the local time by applying the provided clock offset.
 *
 * @param offset The clock offset to apply.
 */
void step_time(double offset)
{
    double ntp_time = (offset);
    prv_update_localTime(ntp_time, 0);
}

/**
 * Adjusts the system time by the given offset.
 *
 * @param offset The clock offset to adjust the time by.
 */
void adjust_time(double offset)
{
    // FreeRTOS_printf(("Adjusting time by:\n"));
    // FreeRTOS_printf_wrapper_double("", offset);
    double ntp_time = (offset);
    prv_update_localTime(offset, 1);
}
