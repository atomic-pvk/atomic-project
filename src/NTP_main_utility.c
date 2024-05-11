#include "NTP_main_utility.h"

#include "NTPTask.h"
#include "NTP_peer.h"

static struct ntp_p *p; /* peer structure pointer */

struct ntp_p *mobilize(uint32_t srcaddr, /* IP source address, change from ipaddr to uint32_t */
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
    memset(p, 0, sizeof(ntp_p));
    p->srcaddr = srcaddr;
    p->dstaddr = dstaddr;
    p->version = version;
    p->hmode = mode;
    p->keyid = keyid;
    p->hpoll = MINPOLL;
    clear(p, X_INIT);
    p->flags = flags;
    return (p);
}

/*
 * find_assoc() - find a matching association
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

void prv_swap_fields(struct ntp_packet *pkt)
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

void create_ntp_r(struct ntp_r *r, ntp_packet *pkt)
{
    r->srcaddr = NTP1_server_IP;  // Set to zero if not known
    r->dstaddr = DSTADDR;         // Set to zero if not known

    // Extract version, leap, and mode from li_vn_mode
    // pkt->li_vn_mode = FreeRTOS_ntohl(pkt->li_vn_mode) << 24;
    r->leap = (pkt->li_vn_mode >> 6) & 0x3;     // Extract bits 0-1
    r->version = (pkt->li_vn_mode >> 3) & 0x7;  // Extract bits 2-4
    r->mode = (pkt->li_vn_mode) & 0x7;          // Extract bits 5-7

    // uint8_t li_vn_mode;      // Eight bits. li, vn, and mode.
    //                          // li.   Two bits.   Leap indicator.
    //                          // vn.   Three bits. Version number of the protocol.
    //                          // mode. Three bits. Client will pick mode 3 for client
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

void recv_packet(ntp_r *r)
{
    ntp_packet pkt;
    int32_t iReturned;

    struct freertos_sockaddr xSourceAddress;
    socklen_t xAddressLength = sizeof(xSourceAddress);

    memset(r, 0, sizeof(ntp_r));  // Clear the receive packet struct

    // FreeRTOS_printf(("Receiving packet...\n"));

    iReturned = FreeRTOS_recvfrom(xSocket, &pkt, sizeof(ntp_packet), 0, &xSourceAddress, &xAddressLength);

    if (iReturned > 0)
    {
        prv_swap_fields(&pkt);

        // FreeRTOS_printf(("Packet received\n"));

        // do conversion
        // ntp_packet -> ntp_r
        create_ntp_r(r, &pkt);
        printTimestamp(r->org, "The original time is: ");
    }
    else
    {
        // FreeRTOS_printf(("Error receiving packet\n"));
    }
}
void prep_xmit(ntp_x *x)
{
    x->leap = s.leap;
    x->stratum = s.stratum;

    // Root delay and dispersion can come from a server or client
    x->rootdelay = s.rootdelay;
    x->rootdisp = s.rootdisp;
    x->refid = s.refid;

    // Assuming tstamp is a type that can be split into seconds and fraction of a second
    x->reftime = s.reftime;
    x->org = x->xmt;
    x->rec = c.localTime;
    gettime(1);
    x->xmt = c.localTime;
}

void prv_translate_ntp_x_to_ntp_packet(const ntp_x *x, ntp_packet *pkt)
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

/*
 * xmit_packet - transmit packet to network
 */
void xmit_packet(ntp_x *x /* transmit packet pointer */
)
{
    /* setup ntp_packet *pkt */
    ntp_packet pkt;  // Allocate memory for the packet

    // set xmit time to current time!
    gettime(0);
    x->xmt = c.localTime;

    prv_translate_ntp_x_to_ntp_packet(x, &pkt);
    prv_swap_fields(&pkt);

    // FreeRTOS_printf(("Sending packet...\n"));

    // Add the association to the table
    if (!assoc_table_add(x->srcaddr, x->mode, x->xmt))
    {
        // FreeRTOS_printf(("Error adding association to table\n"));
    }

    int32_t iReturned;
    socklen_t xAddressLength = sizeof(xDestinationAddress);

    iReturned = FreeRTOS_sendto(xSocket, &pkt, sizeof(ntp_packet), 0, &xDestinationAddress, xAddressLength);

    if (iReturned == sizeof(ntp_packet))
    {
        // FreeRTOS_printf(("Sent packet\n"));
    }
    else
    {
        // FreeRTOS_printf(("Failed to send packet\n"));
    }
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
 * step_time() - step system time to given offset value
 */
void step_time(double offset /* clock offset */
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
void adjust_time(double offset /* clock offset */
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
