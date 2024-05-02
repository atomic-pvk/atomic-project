#include "NTPTask.h"
#include "NTP_main_utility.h"

static struct ntp_p *p; /* peer structure pointer */

struct ntp_p *mobilize(
    uint32_t srcaddr, /* IP source address, change from ipaddr to uint32_t */
    uint32_t dstaddr, /* IP destination address, change from ipaddr to uint32_t */
    int version,      /* version */
    int mode,         /* host mode */
    int keyid,        /* key identifier */
    int flags         /* peer flags */
)
{
    struct ntp_p *p; /* peer process pointer */

    /*
     * Allocate and initialize association memory
     */
    p = malloc(sizeof(struct ntp_p));
    p->srcaddr = srcaddr;
    p->dstaddr = dstaddr;
    p->version = version;
    p->hmode = mode;
    p->keyid = keyid;
    p->hpoll = MINPOLL;
    // clear(p, X_INIT);
    p->flags = flags;
    return (p);
}

/*
 * find_assoc() - find a matching association
 */
struct ntp_p /* peer structure pointer or NULL */
    *
    find_assoc(
        struct ntp_r *r /* receive packet pointer */
    )
{
    struct ntp_p *p; /* dummy peer structure pointer */
    p = malloc(sizeof(struct ntp_p));

    FreeRTOS_printf(("searching for Association\n"));

    for (int i = 0; i < assoc_table->size; i++)
    {
        Assoc_info assoc = assoc_table->entries[i];
        if (r->srcaddr == assoc.srcaddr && r->mode == assoc.hmode)
        {
            FreeRTOS_printf(("Association found\n"));
            p->srcaddr = assoc.srcaddr;
            p->hmode = 3;
            p->pmode = assoc.hmode;

            FreeRTOS_printf(("values: %d %d\n", p->srcaddr, p->hmode));
            return (p);
        }
    }
    return (NULL);
}

/*
 * md5() - compute message digest
 */
digest
md5(
    int keyid /* key identifier */
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

// A.3.  Kernel Input/Output Interface

void FreeRTOS_ntohl_array_32(uint32_t *array, size_t length)
{
    for (size_t i = 0; i < length; i++)
    {
        array[i] = FreeRTOS_ntohl(array[i]);
    }
}

void create_ntp_r(struct ntp_r *r, ntp_packet *pkt, uint64_t time_in_ms)
{
    r->srcaddr = NTP1_server_IP; // Set to zero if not known
    r->dstaddr = 0;              // Set to zero if not known

    FreeRTOS_printf(("\n\nhex debugging: %X\n\n", pkt->li_vn_mode));

    // 11100111
    // 00100100 (correct!)

    // Extract version, leap, and mode from li_vn_mode
    // pkt->li_vn_mode = FreeRTOS_ntohl(pkt->li_vn_mode) << 24;
    r->leap = (pkt->li_vn_mode >> 6) & 0x3;    // Extract bits 0-1
    r->version = (pkt->li_vn_mode >> 3) & 0x7; // Extract bits 2-4
    r->mode = (pkt->li_vn_mode) & 0x7;         // Extract bits 5-7

    FreeRTOS_printf(("\n\nhex debugging: %X\n\n", r->mode));

    // LLVVVMMM
    // LLVVVMMM & 111 = MMM
    // LLVVVMMM >> 3 & 111= VVV
    // LLVVVMMM >> 6 & 11 = LL

    // r->version = (pkt->li_vn_mode >> 3) & 0x07; // Extract bits 3-5
    // r->leap = (pkt->li_vn_mode >> 6) & 0x03;    // Extract bits 6-7
    // r->mode = pkt->li_vn_mode & 0x07;           // Extract bits 0-2

    // uint8_t li_vn_mode;      // Eight bits. li, vn, and mode.
    //                          // li.   Two bits.   Leap indicator.
    //                          // vn.   Three bits. Version number of the protocol.
    //                          // mode. Three bits. Client will pick mode 3 for client
    r->stratum = ((char)pkt->stratum);
    r->poll = (char)pkt->poll;
    r->precision = (s_char)pkt->precision;

    uint32_t temp[12]; // 384-bit data broken down into 32-bit chunks
    memcpy(temp, pkt, sizeof(ntp_packet));
    FreeRTOS_ntohl_array_32(temp, 12);
    memcpy(pkt, temp, sizeof(ntp_packet)); // Copy the received packet to the receive packet struct

    r->rootdelay = (tdist)pkt->rootDelay;
    r->rootdisp = (tdist)pkt->rootDispersion;

    r->refid = (char)pkt->refId; // May require handling depending on refId's nature

    // Combine seconds and fractions into a single 64-bit NTP timestamp
    r->reftime = ((tstamp)pkt->refTm_s << 32) | pkt->refTm_f;
    r->org = ((tstamp)pkt->origTm_s << 32) | pkt->origTm_f;
    r->rec = ((tstamp)pkt->rxTm_s << 32) | pkt->rxTm_f;
    r->xmt = ((tstamp)pkt->txTm_s << 32) | pkt->txTm_f;

    // Set crypto fields to 0 or default values
    r->keyid = 0;
    r->mac = 0;          // Zero out the MAC digest
    r->dst = time_in_ms; // Set the timestamp to the ms passed since vTaskStartScheduler started
}

/*
 * Kernel interface to transmit and receive packets.  Details are
 * deliberately vague and depend on the operating system.
 *
 * recv_packet - receive packet from network
 */
struct ntp_r /* receive packet pointer*/
    *
    recv_packet()
{
    ntp_packet *pkt = malloc(sizeof(ntp_packet)); // Allocate memory for the packet
    memset(pkt, 0, sizeof(ntp_packet));           // Clear the packet struct
    int32_t iReturned;
    struct freertos_sockaddr xSourceAddress;
    socklen_t xAddressLength = sizeof(xSourceAddress);
    unsigned char bufferRecv[sizeof(ntp_packet)];

    FreeRTOS_printf(("Receiving packet...\n"));

    iReturned = FreeRTOS_recvfrom(xSocket,
                                  bufferRecv,
                                  sizeof(ntp_packet),
                                  0,
                                  &xSourceAddress,
                                  &xAddressLength);

    if (iReturned > 0)
    {
        // TODO: Clock local time when packet is recieved (tstamp dst)
        TickType_t time_in_ms = xTaskGetTickCount();

        struct ntp_r *r = malloc(sizeof(ntp_r)); // Allocate memory for the receive packet
        memset(r, 0, sizeof(ntp_r));             // Clear the receive packet struct

        // Flip the bits

        uint32_t temp[12]; // 384-bit data broken down into 32-bit chunks
        memcpy(temp, bufferRecv, sizeof(ntp_packet));
        // FreeRTOS_ntohl_array_32(temp, 12);
        memcpy(pkt, temp, sizeof(ntp_packet)); // Copy the received packet to the receive packet struct

        FreeRTOS_printf(("Packet received\n"));

        // do conversion
        // ntp_packet -> ntp_r
        create_ntp_r(r, pkt, (uint64_t)time_in_ms);

        // Add the association to the table
        if (!assoc_table_add(assoc_table, r->srcaddr, r->mode))
        {
            FreeRTOS_printf(("Error adding association to table\n"));
            r = NULL;
        }

        return r;
    }
    else
    {
        FreeRTOS_printf(("Error receiving packet\n"));

        return NULL;
    }
}

/*
 * xmit_packet - transmit packet to network
 */
void xmit_packet(
    struct ntp_x *x /* transmit packet pointer */
)
{
    /* setup ntp_packet *pkt */
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

    /* Send the string to the UDP socket.  ulFlags is set to 0, so the standard
    semantics are used.  That means the data from cString[] is copied
    into a network buffer inside FreeRTOS_sendto(), and cString[] can be
    reused as soon as FreeRTOS_sendto() has returned. */
    FreeRTOS_printf(("Sending packet...\n"));

    int32_t iReturned;
    socklen_t xAddressLength = sizeof(xDestinationAddress);

    iReturned = FreeRTOS_sendto(xSocket,
                                buffer,
                                sizeof(buffer),
                                0,
                                &xDestinationAddress,
                                xAddressLength);

    if (iReturned == sizeof(buffer))
    {
        FreeRTOS_printf(("Sent packet\n"));
    }
    else
    {
        FreeRTOS_printf(("Failed to send packet\n"));
    }

    /* send packet x */
}

// A.4.  Kernel System Clock Interface

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

/*
 * get_time - read system time and convert to NTP format
 */
tstamp
get_time()
{
    struct timeval unix_time;

    /*
     * There are only two calls on this routine in the program.  One
     * when a packet arrives from the network and the other when a
     * packet is placed on the send queue.  Call the kernel time of
     * day routine (such as gettimeofday()) and convert to NTP
     * format.
     */
    // gettimeofday(&unix_time, NULL);
    return (U2LFP(unix_time));
}

/*
 * step_time() - step system time to given offset value
 */
void step_time(
    double offset /* clock offset */
)
{
    struct timeval unix_time;
    tstamp time;

    /*
     * Convert from double to native format (signed) and add to the
     * current time.  Note the addition is done in native format to
     * avoid overflow or loss of precision.
     */
    // gettimeofday(&unix_time, NULL);
    time = D2LFP(offset) + U2LFP(unix_time);
    unix_time.tv_sec = time >> 32;
    unix_time.tv_usec = (long)(((time - unix_time.tv_sec) << 32) / FRAC * 1e6);
    // settimeofday(&unix_time, NULL);
}

/*
 * adjust_time() - slew system clock to given offset value
 */
void adjust_time(
    double offset /* clock offset */
)
{
    struct timeval unix_time;
    tstamp time;

    /*
     * Convert from double to native format (signed) and add to the
     * current time.
     */
    time = D2LFP(offset);
    unix_time.tv_sec = time >> 32;
    unix_time.tv_usec = (long)(((time - unix_time.tv_sec) << 32) / FRAC * 1e6);
    // adjtime(&unix_time, NULL);
}