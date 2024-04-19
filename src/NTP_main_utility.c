#include "UDPEchoClientDemo.h"
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

    /*
     * Search association table for matching source
     * address, source port and mode.
     */
    while (/* all associations */ 0)
    {
        if (r->srcaddr == p->srcaddr && r->mode == p->hmode)
            return (p);
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
    FreeRTOS_printf(("\n\n in here 1\n\n"));
    ntp_packet *pkt = malloc(sizeof(ntp_packet)); // Allocate memory for the packet
    memset(pkt, 0, sizeof(ntp_packet));           // Clear the packet struct
    int32_t iReturned;
    struct freertos_sockaddr xSourceAddress;
    socklen_t xAddressLength = sizeof(xSourceAddress);

    FreeRTOS_printf(("\n\n in here \n\n"));

    iReturned = FreeRTOS_recvfrom(xSocket,
                                  &pkt,
                                  sizeof(pkt),
                                  0,
                                  &xSourceAddress,
                                  &xAddressLength);

    if (iReturned > 0)
    {
        struct ntp_r *r = malloc(sizeof(ntp_r)); // Allocate memory for the receive packet
        memset(r, 0, sizeof(ntp_r));             // Clear the receive packet struct

        memcpy(r, pkt, sizeof(ntp_packet)); // Copy the received packet to the receive packet struct

        return r;
    }
    else
    {
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

    /* NOTE: FreeRTOS_bind() is not called.  This will only work if
    ipconfigALLOW_SOCKET_SEND_WITHOUT_BIND is set to 1 in FreeRTOSIPConfig.h. */

    // print the fucking buffer

    /* Create the string that is sent. */
    // sprintf(cString,
    //         "Standard send message number %lurn",
    //         ulCount);

    /* Send the string to the UDP socket.  ulFlags is set to 0, so the standard
    semantics are used.  That means the data from cString[] is copied
    into a network buffer inside FreeRTOS_sendto(), and cString[] can be
    reused as soon as FreeRTOS_sendto() has returned. */
    FreeRTOS_printf(("\n\n Sending packet... \n\n"));

    int32_t iReturned;

    iReturned = FreeRTOS_sendto(xSocket,
                                buffer,
                                sizeof(buffer),
                                0,
                                xDestinationAddress,
                                sizeof(xDestinationAddress));

    if (iReturned == sizeof(buffer))
    {
        FreeRTOS_printf(("\n\n Sent packet \n\n"));
    }
    else
    {
        FreeRTOS_printf(("\n\n Failed to send packet \n\n"));
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