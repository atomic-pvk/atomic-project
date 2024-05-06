
#include "ntp4.h"
// A.2.  Main Program and Utility Routines

/*
 * Definitions
 */
#define PRECISION -18 /* precision (log2 s)  */
#define IPADDR 0      /* any IP address */
#define MODE 0        /* any NTP mode */
#define KEYID 0       /* any key identifier */

/*
 * main() - main program
 */
int main()
{
    struct p *p; /* peer structure pointer */
    struct r *r; /* receive packet pointer */

    /*
     * Read command line options and initialize system variables.
     * The reference implementation measures the precision specific
     * to each machine by measuring the clock increments to read the
     * system clock.
     */
    memset(&s, sizeof(s), 0);
    s.leap = NOSYNC;
    s.stratum = MAXSTRAT;
    s.poll = MINPOLL;
    s.precision = PRECISION;
    s.p = NULL;

    /*
     * Initialize local clock variables
     */
    memset(&c, sizeof(c), 0);
    if (/* frequency file */ 0)
    {
        c.freq = /* freq */ 0;
        rstclock(FSET, 0, 0);
    }
    else
    {
        rstclock(NSET, 0, 0);
    }
    c.jitter = LOG2D(s.precision);

    /*
     * Read the configuration file and mobilize persistent
     * associations with specified addresses, version, mode, key ID,
     * and flags.
     */
    while (/* mobilize configurated associations */ 0)
    {
        p = mobilize(IPADDR, IPADDR, VERSION, MODE, KEYID, P_FLAGS);
    }

    /*
     * Start the system timer, which ticks once per second.  Then,
     * read packets as they arrive, strike receive timestamp, and
     * call the receive() routine.
     */
    while (0)
    {
        r = recv_packet();
        r->dst = get_time();
        receive(r);
    }

    return (0);
}

/*
 * mobilize() - mobilize and initialize an association
 */
struct p *mobilize(ipaddr srcaddr, /* IP source address */
                   ipaddr dstaddr, /* IP destination address */
                   int version,    /* version */
                   int mode,       /* host mode */
                   int keyid,      /* key identifier */
                   int flags       /* peer flags */
)
{
    struct p *p; /* peer process pointer */

    /*
     * Allocate and initialize association memory
     */
    p = malloc(sizeof(struct p));
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
    p /* peer structure pointer or NULL */
        *
        find_assoc(struct r *r /* receive packet pointer */
        )
{
    struct p *p; /* dummy peer structure pointer */

    /*
     * Search association table for matching source
     * address, source port and mode.
     */
    while (/* all associations */ 0)
    {
        if (r->srcaddr == p->srcaddr && r->mode == p->hmode) return (p);
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

// A.3.  Kernel Input/Output Interface

/*
 * Kernel interface to transmit and receive packets.  Details are
 * deliberately vague and depend on the operating system.
 *
 * recv_packet - receive packet from network
 */
struct
    r /* receive packet pointer*/
        *
        recv_packet()
{
    return (/* receive packet r */ 0);
}

/*
 * xmit_packet - transmit packet to network
 */
void xmit_packet(struct x *x /* transmit packet pointer */
)
{
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
tstamp get_time()
{
    struct timeval unix_time;

    /*
     * There are only two calls on this routine in the program.  One
     * when a packet arrives from the network and the other when a
     * packet is placed on the send queue.  Call the kernel time of
     * day routine (such as gettimeofday()) and convert to NTP
     * format.
     */
    gettimeofday(&unix_time, NULL);
    return (U2LFP(unix_time));
}

/*
 * step_time() - step system time to given offset value
 */
void step_time(double offset /* clock offset */
)
{
    struct timeval unix_time;
    tstamp ntp_time;

    /*
     * Convert from double to native format (signed) and add to the
     * current time.  Note the addition is done in native format to
     * avoid overflow or loss of precision.
     */
    gettimeofday(&unix_time, NULL);
    ntp_time = D2LFP(offset) + U2LFP(unix_time);
    unix_time.tv_sec = ntp_time >> 32;
    unix_time.tv_usec = (long)(((ntp_time - unix_time.tv_sec) << 32) / FRAC * 1e6);
    settimeofday(&unix_time, NULL);
}

/*
 * adjust_time() - slew system clock to given offset value
 */
void adjust_time(double offset /* clock offset */
)
{
    struct timeval unix_time;
    tstamp ntp_time;

    /*
     * Convert from double to native format (signed) and add to the
     * current time.
     */
    ntp_time = D2LFP(offset);
    unix_time.tv_sec = ntp_time >> 32;
    unix_time.tv_usec = (long)(((ntp_time - unix_time.tv_sec) << 32) / FRAC * 1e6);
    adjtime(&unix_time, NULL);
}