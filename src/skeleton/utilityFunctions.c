// utility functions
#include "ntp4.h"
/*
 * Definitions
 */
#define PRECISION -18 /* precision (log2 s) */
#define IPADDR 0      /* any IP address */
#define MODE 0        /* any NTP mode */
#define KEYID 0       /* any key identifier */

/*
 * mobilize() - mobilize and initialize an association
 */
struct p *mobilize(
    ipaddr srcaddr, /* IP source address */
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
        p->srcport = PORT;
        p->dstaddr = dstaddr;
        p->dstport = PORT;
        p->version = version;
        p->mode = mode;
        p->keyid = keyid;
        p->hpoll = MINPOLL;
        clear(p, X_INIT);
        p->flags == flags;
        return (p);
}

/*
 * These are used by the clear() routine
 */
#define BEGIN_CLEAR(p) ((char *)&((p)->begin_clear))
#define END_CLEAR(p) ((char *)&((p)->end_clear))
#define LEN_CLEAR (END_CLEAR((struct p *)0) - \
                   BEGIN_CLEAR((struct p *)0))

/*
 * clear() - reinitialize for persistent association, demobilize
 * for ephemeral association.
 */
void clear(
    struct p *p, /* peer structure pointer */
    int kiss     /* kiss code */
)
{
        int i;

        /*
         * The first thing to do is return all resources to the bank.
         * Typical resources are not detailed here, but they include
         * dynamically allocated structures for keys, certificates, etc. * If an ephemeral association and not initialization, return
         * the association memory as well.
         */
        /* return resources */
        if (s.p == p)
                s.p = NULL;
        if (kiss != X_INIT && (p->flags & P_EPHEM))
        {
                free(p);
                return;
        }

        /*
         * Initialize the association fields for general reset.
         */
        memset(BEGIN_CLEAR(p), LEN_CLEAR, 0);
        p->leap = NOSYNC;
        p->stratum = MAXSTRAT;
        p->ppoll = MAXPOLL;
        p->hpoll = MINPOLL;
        p->disp = MAXDISP;
        p->jitter = LOG2D(s.precision);
        p->refid = kiss;
        for (i = 0; i < NSTAGE; i++)
                p->f[i].disp = MAXDISP;

        /*
         * Randomize the first poll just in case thousands of broadcast
         * clients have just been stirred up after a long absence of the
         * broadcast server. */
        p->last = p->t = c.t;
        p->next = p->last + (random() & ((1 << MINPOLL) - 1));
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
         * Compute a keyed cryptographic message digest. The key
         * identifier is associated with a key in the local key cache.
         * The key is prepended to the packet header and extension fieds
         * and the result hashed by the MD5 algorithm as described in
         * RFC-1321. Return a MAC consisting of the 32-bit key ID
         * concatenated with the 128-bit digest.
         */
        return (/* MD5 digest */ 0);
}

/*
 * There are three time formats: native (Unix), NTP and floating double.
 * The get_time() routine returns the time in NTP long format. The Unix
 * routines expect arguments as a structure of two signed 32-bit words
 * in seconds and microseconds (timeval) or nanoseconds (timespec). The
 * step_time() and adjust_time() routines expect signed arguments in
 * floating double. The simplified code shown here is for illustration
 * only and has not been verified.
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
         * There are only two calls on this routine in the program. One
         * when a packet arrives from the network and the other when a
         * packet is placed on the send queue. Call the kernel time of
         * day routine (such as gettimeofday()) and convert to NTP format.
         */
        gettimeofday(&unix_time, NULL);
        return ((unix_time.tv_sec + JAN_1970) * 0x100000000L +
                (unix_time.tv_usec * 0x100000000L) / 1000000);
}

/*
 * step_time() - step system time to given offset valuet
 */
void step_time(
    double offset /* clock offset */
)
{
        struct timeval unix_time;
        tstamp ntp_time;
        /*
         * Convert from double to native format (signed) and add to the
         * current time. Note the addition is done in native format to
         * avoid overflow or loss of precision.
         */
        ntp_time = D2LFP(offset);
        gettimeofday(&unix_time, NULL);
        unix_time.tv_sec += ntp_time / 0x100000000L;
        unix_time.tv_usec += ntp_time % 0x100000000L;
        unix_time.tv_sec += unix_time.tv_usec / 1000000;
        unix_time.tv_usec %= 1000000;
        settimeofday(&unix_time, NULL);
}

/*
 * adjust_time() - slew system clock to given offset value
 */
void adjust_time(
    double offset /* clock offset */
)
{
        struct timeval unix_time;
        tstamp ntp_time;
        /*
         * Convert from double to native format (signed) and add to the
         * current time.
         */
        ntp_time = D2LFP(offset);
        unix_time.tv_sec = ntp_time / 0x100000000L;
        unix_time.tv_usec = ntp_time % 0x100000000L;
        unix_time.tv_sec += unix_time.tv_usec / 1000000;
        unix_time.tv_usec %= 1000000;
        adjtime(&unix_time, NULL);
}
