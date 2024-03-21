// @author Hugo Larsson Wilhelmsson and Jack Gugolz

// 2. MAIN PROGRAM AND UTILITY ROUTINES
#include "ntp4.h"
/*
 * Definitions
 */
#define PRECISION -18 /* precision (log2 s) */
#define IPADDR 0      /* any IP address */
#define MODE 0        /* any NTP mode */
#define KEYID 0       /* any key identifier */

/*
 * main() - main program
 */
int main() /* main program */
{
        struct p *p; /* peer structure pointer */
        struct r *r; /* receive packet pointer */

        /*
         * Read command line options and initialize system variables.
         * The reference implementation measures the precision specific
         * to each machine by measuring the clock increments to read the
         * system clock. */
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
         * associations with spcified addresses, version, mode, key ID * and flags.
         */
        while (/* mobilize configurated associations */ 0)
        {
                p = mobilize(IPADDR, IPADDR, VERSION, MODE, KEYID,
                             P_FLAGS);
        }

        /*
         * Start the system timer, which ticks once per second. Then
         * read packets as they arrive, strike receive timestamp and
         * call the receive() routine.
         */
        while (0)
        {
                r = recv_packet();
                r->dst = get_time();
                receive(r);
        }
}

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

// 3 KERNEL INPUT/OUTPUT INTERFACE

/*
 * Kernel interface to transmit and receive packets. Details are
 * deliberately vague and depend on the operating system. *
 * recv_packet - receive packet from network
 */
struct r /* receive packet pointer*/
    *
    recv_packet()
{
        return (/* receive packet r */ 0);
}

/*
 * xmit_packet - transmit packet to network
 */
void xmit_packet(
    struct x *x /* transmit packet pointer */
)
{
        /* send packet x */
}

// 4 KERNEL SYSTEM CLOCK INTERFACE

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



// 6. System Process

#include "ntp4.h"

        // 6.1 clock_select()

        /*
         * clock_select() - find the best clocks
         */
        void
        clock_select();
        {
                struct p *p, *osys;      /* peer structure pointers */
                double low, high;        /* correctness interval extents */
                int allow, found, chime; /* used by intersecion algorithm */
                int n, i, j;
                /*
                 * We first cull the falsetickers from the server population,
                 * leaving only the truechimers. The correctness interval for
                 * association p is the interval from offset - root_dist() to
                 * offset + root_dist(). The object of the game is to find a
                 * majority clique; that is, an intersection of correctness
                 * intervals numbering more than half the server population.
                 *
                 * First construct the chime list of tuples (p, type, edge) as
                 * shown below, then sort the list by edge from lowest to
                 * highest.
                 */
                osys = s.p;
                s.p = NULL;
                n = 0;
                while (accept(p))
                {
                        s.m[n].p = p;
                        s.m[n].type = +1;
                        s.m[n].edge = p->offset + root_dist(p);
                        n++;
                        s.m[n].p = p;
                        s.m[n].type = 0;
                        s.m[n].edge = p->offset;
                        n++;
                        s.m[n].p = p;
                        s.m[n].type = -1;
                        s.m[n].edge = p->offset - root_dist(p);
                        n++;
                }
                /*
                 * Find the largest contiguous intersection of correctness
                 * intervals. Allow is the number of allowed falsetickers; found
                 * is the number of midpoints. Note that the edge values are
                 * limited to the range +-(2 ^ 30) < +-2e9 by the timestamp
                 * calculations.
                 */
                low = 2e9;
                high = -2e9;
                for (allow = 0; 2 * allow < n; allow++)
                {
                        /*
                         * Scan the chime list from lowest to highest to find
                         * the lower endpoint.
                         */
                        found = 0;
                        chime = 0;
                        for (i = 0; i < n; i++)
                        {
                                chime -= s.m[i].type;
                                if (chime >= n - found)
                                {
                                        low = s.m[i].edge;
                                        break;
                                }
                                if (s.m[i].type == 0)
                                        found++;
                        }
                        /*
                         * Scan the chime list from highest to lowest to find
                         * the upper endpoint.
                         */
                        chime = 0;
                        for (i = n - 1; i >= 0; i--)
                        {
                                chime += s.m[i].type;
                                if (chime >= n - found)
                                {
                                        high = s.m[i].edge;
                                        break;
                                }
                                if (s.m[i].type == 0)
                                        found++;
                        }
                        /*
                         * If the number of midpoints is greater than the number
                         * of allowed falsetickers, the intersection contains at
                         * least one truechimer with no midpoint. If so,
                         * increment the number of allowed falsetickers and go
                         * around again. If not and the intersection is
                         * nonempty, declare success.
                         */
                        if (found > allow)
                                continue;
                        if (high > low)
                                break;
                }
                /*
                 * Clustering algorithm. Construct a list of survivors (p,
                 * metric) from the chime list, where metric is dominated first
                 * by stratum and then by root distance. All other things being
                 * equal, this is the order of preference.
                 */
                n = 0;
                for (i = 0; i < n; i++)
                {
                        if (s.m[i].edge < low || s.m[i].edge > high)
                                continue;
                        p = s.m[i].p;
                        s.v[n].p = p;
                        s.v[n].metric = MAXDIST * p->stratum + root_dist(p);
                        n++;
                }
                /*
                 * There must be at least NSANE survivors to satisfy the
                 * correctness assertions. Ordinarily, the Byzantine criteria
                 * require four, susrvivors, but for the demonstration here, one
                 * is acceptable.
                 */
                if (n == NSANE)
                        return;
                /*
                 * For each association p in turn, calculate the selection
                 * jitter p->sjitter as the square root of the sum of squares
                 * (p->offset - q->offset) over all q associations. The idea is
                 * to repeatedly discard the survivor with maximum selection
                 * jitter until a termination condition is met.
                 */
                while (1)
                {
                        struct p *p, *q, *qmax; /* peer structure pointers */
                        double max, min, dtemp;
                        max = -2e9;
                        min = 2e9;
                        for (i = 0; i < n; i++)
                        {
                                p = s.v[i].p;
                                if (p->jitter < min)
                                        min = p->jitter;
                                dtemp = 0;
                                for (j = 0; j < n; j++)
                                {
                                        q = s.v[j].p;
                                        dtemp += SQUARE(p->offset - q->offset);
                                }
                                dtemp = SQRT(dtemp);
                                if (dtemp > max)
                                {
                                        max = dtemp;
                                        qmax = q;
                                }
                        }
                        /*
                         * If the maximum selection jitter is less than the
                         * minimum peer jitter, then tossing out more survivors
                         * will not lower the minimum peer jitter, so we might
                         * as well stop. To make sure a few survivors are left
                         * for the clustering algorithm to chew on, we also stop
                         * if the number of survivors is less than or equal to
                         * NMIN (3).
                         */
                        if (max < min || n <= NMIN)
                                break;
                        /*
                         * Delete survivor qmax from the list and go around
                         * again.
                         */
                        n--;
                }
                /*
                 * Pick the best clock. If the old system peer is on the list
                 * and at the same stratum as the first survivor on the list,
                 * then don’t do a clock hop. Otherwise, select the first
                 * survivor on the list as the new system peer.
                 */
                if (osys->stratum == s.v[0].p->stratum)
                        s.p = osys;
                else
                        s.p = s.v[0].p;
                clock_update(s.p);
        }

        // 6.2 root_dist()

        /*
         * root_dist() - calculate root distance
         */
        double
            root_dist(
                struct p * p /* peer structure pointer */
            );
        {
                /*
                 * The root synchronization distance is the maximum error due to
                 * all causes of the local clock relative to the primary server.
                 * It is defined as half the total delay plus total dispersion
                 * plus peer jitter.
                 */
                return (max(MINDISP, p->rootdelay + p->delay) / 2 +
                        p->rootdisp + p->disp + PHI * (c.t - p->t) + p->jitter);
        }

        // 6.3 accept()

        /*
         * accept() - test if association p is acceptable for synchronization
         */
        int
            accept(
                struct p * p /* peer structure pointer */
            );
        {
                /*
                 * A stratum error occurs if (1) the server has never been
                 * synchronized, (2) the server stratum is invalid.
                 */
                if (p->leap == NOSYNC || p->stratum >= MAXSTRAT)
                        return (FALSE);
                /*
                 * A distance error occurs if the root distance exceeds the
                 * distance threshold plus an increment equal to one poll
                 * interval.
                 */
                if (root_dist(p) > MAXDIST + PHI * LOG2D(s.poll))
                        return (FALSE);
                /*
                 * A loop error occurs if the remote peer is synchronized to the
                 * local peer or the remote peer is synchronized to the current
                 * system peer. Note this is the behavior for IPv4; for IPv6 the
                 * MD5 hash is used instead.
                 */
                if (p->refid == p->dstaddr || p->refid == s.refid)
                        return (FALSE);
                /*
                 * An unreachable error occurs if the server is unreachable.
                 */
                if (p->reach == 0)
                        return (FALSE);
                return (TRUE);
        }

        // 6.4 clock_update()

        /*
         * clock_update() - update the system clock
         */
        void
            clock_update(
                struct p * p /* peer structure pointer */
            );
        {
                double dtemp;
                /*
 * If this is an old update, for instance as the result of a
 * system peer change, avoid it. We never use an old sample or
 * the same sample twice.
 *
if (s.t >= p->t)
return;
/*
 * Combine the survivor offsets and update the system clock; the
 * local_clock() routine will tell us the good or bad news.
 */
                s.t = p->t;
                clock_combine();
                switch (local_clock(p, s.offset))
                {
                /*
                 * The offset is too large and probably bogus. Complain to the
                 * system log and order the operator to set the clock manually
                 * within PANIC range. The reference implementation includes a
                 * command line option to disable this check and to change the
                 * panic threshold from the default 1000 s as required.
                 */
                case PANIC:
                        exit(0);
                /*
                 * The offset is more than the step threshold (0.125 s by
                 * default). After a step, all associations now have
                 * inconsistent time valurs, so they are reset and started
                 * fresh. The step threshold can be changed in the reference
                 * implementation in order to lessen the chance the clock might
                 * be stepped backwards. However, there may be serious
                 * consequences, as noted in the white papers at the NTP project
                 * site.
                 */
                case STEP:
                        while (/* all associations */ 0)
                                clear(p, X_STEP);
                        s.stratum = MAXSTRAT;
                        s.poll = MINPOLL;
                        break;
                /*
                 * The offset was less than the step threshold, which is the
                 * normal case. Update the system variables from the peer
                 * variables. The lower clamp on the dispersion increase is to
                 * avoid timing loops and clockhopping when highly precise
                 * sources are in play. The clamp can be changed from the
                 * default .01 s in the reference implementation.
                 */
                case SLEW:
                        s.leap = p->leap;
                        s.stratum = p->stratum + 1;
                        s.refid = p->refid;
                        s.reftime = p->reftime;
                        s.rootdelay = p->rootdelay + p->delay;
                        dtemp = SQRT(SQUARE(p->jitter) + SQUARE(s.jitter));
                        dtemp += max(p->disp + PHI * (c.t - p->t) +
                                         fabs(p->offset),
                                     MINDISP);
                        s.rootdisp = p->rootdisp + dtemp;
                        break;
                /*
                 * Some samples are discarded while, for instance, a direct
                 * frequency measurement is being made.
                 */
                case IGNORE:
                        break;
                }
        }

        // 6.5 clock_combine()

        /*
         * clock_combine() - combine offsets
         */
        void
        clock_combine();
        {
                struct p *p; /* peer structure pointer */
                double x, y, z, w;
                int i;
                /*
                 * Combine the offsets of the clustering algorithm survivors
                 * using a weighted average with weight determined by the root
                 * distance. Compute the selection jitter as the weighted RMS
                 * difference between the first survivor and the remaining
                 * survivors. In some cases the inherent clock jitter can be
                 * reduced by not using this algorithm, especially when frequent
                 * clockhopping is involved. The reference implementation can be
                 * configured to avoid this algorithm by designating a preferred
                 * peer.
                 */
                y = z = w = 0;
                for (i = 0; s.v[i].p != NULL; i++)
                {
                        p = s.v[i].p;
                        x = root_dist(p);
                        y += 1 / x;
                        z += p->offset / x;
                        w += SQUARE(p->offset - s.v[0].p->offset) / x;
                }
                s.offset = z / y;
                s.jitter = SQRT(w / y);
        }

        // 6.6 local_clock()

#include "ntp4.h"
        /*
         * Constants
         */
#define STEPT .128      /* step threshold (s) */
#define WATCH 900       /* stepout threshold (s) */
#define PANICT 1000     /* panic threshold (s) */
#define PLL 65536       /* PLL loop gain */
#define FLL MAXPOLL + 1 /* FLL loop gain */
#define AVG 4           /* parameter averaging constant */
#define ALLAN 1500      /* compromise Allan intercept (s) */
#define LIMIT 30        /* poll-adjust threshold */
#define MAXFREQ 500e-6  /* maximum frequency tolerance (s/s) */
#define PGATE 4         /* poll-adjust gate */
        /*
         * local_clock() - discipline the local clock
         */
        int /* return code */
        local_clock(
            struct p * p, /* peer structure pointer */
            double offset /* clock offset from combine() */
        );
        {
                int state;   /* clock discipline state */
                double freq; /* frequency */
                double mu;   /* interval since last update */
                int rval;
                double etemp, dtemp;
                /*
                 * If the offset is too large, give up and go home.
                 */
                if (fabs(offset) > PANICT)
                        return (PANIC);
                /*
                 * Clock state machine transition function. This is where the
                 * action is and defines how the system reacts to large time
                 * and frequency errors. There are two main regimes: when the
                 * offset exceeds the step threshold and when it does not.
                 */
                rval = SLEW;
                mu = p->t - s.t;
                freq = 0;
                if (fabs(offset) > STEPT)
                {
                        switch (c.state)
                        {
                        /*
                         * In S_SYNC state we ignore the first outlyer amd
                         * switch to S_SPIK state.
                         */
                        case SYNC:
                                state = SPIK;
                                return (rval);
                                /*
                                 * In S_FREQ state we ignore outlyers and inlyers. At
                                 * the first outlyer after the stepout threshold,
                                 * compute the apparent frequency correction and step
                                 * the time.
                                 */
                        case FREQ:
                                if (mu < WATCH)
                                        return (IGNORE);
                                freq = (offset - c.base - c.offset) / mu;
                        /* fall through to S_SPIK */
                        /*
                         * In S_SPIK state we ignore succeeding outlyers until
                         * either an inlyer is found or the stepout threshold is
                         * exceeded.
                         */
                        case SPIK:
                                if (mu < WATCH)
                                        return (IGNORE);
                        /* fall through to default */
                        /*
                         * We get here by default in S_NSET and S_FSET states
                         * and from above in S_FREQ state. Step the time and
                         * clamp down the poll interval.
                         *
                         * In S_NSET state an initial frequency correction is
                         * not available, usually because the frequency file has
                         * not yet been written. Since the time is outside the
                         * capture range, the clock is stepped. The frequency
                         * will be set directly following the stepout interval.
                         *
                         * In S_FSET state the initial frequency has been set
                         * from the frequency file. Since the time is outside
                         * the capture range, the clock is stepped immediately,
                         * rather than after the stepout interval. Guys get
                         * nervous if it takes 17 minutes to set the clock for
                         * the first time.
                         *
                         * In S_SPIK state the stepout threshold has expired and
                         * the phase is still above the step threshold. Note
                         * that a single spike greater than the step threshold
                         * is always suppressed, even at the longer poll
                         * intervals.
                         */
                        default:
                                /*
                                 * This is the kernel set time function, usually
                                 * implemented by the Unix settimeofday() system
                                 * call.
                                 * */
                                step_time(offset);
                                c.count = 0;
                                rval = STEP;
                                if (state == NSET)
                                {
                                        rstclock(FREQ, p->t, 0);
                                        return (rval);
                                }
                                break;
                        }
                        rstclock(SYNC, p->t, 0);
                }
                else
                {
                        /*
                         * Compute the clock jitter as the RMS of exponentially
                         * weighted offset differences. This is used by the
                         * poll-adjust code.
                         */
                        etemp = SQUARE(c.jitter);
                        dtemp = SQUARE(max(fabs(offset - c.last),
                                           LOG2D(s.precision)));
                        c.jitter = SQRT(etemp + (dtemp - etemp) / AVG);
                        switch (c.state)
                        {
                        /*
                         * In S_NSET state this is the first update received and
                         * the frequency has not been initialized. The first
                         * thing to do is directly measure the oscillator
                         * frequency.
                         */
                        case NSET:
                                c.offset = offset;
                                rstclock(FREQ, p->t, offset);
                                return (IGNORE);
                        /*
                         * In S_FSET state this is the first update and the
                         * frequency has been initialized. Adjust the phase, but
                         * don’t adjust the frequency until the next update.
                         */
                        case FSET:
                                c.offset = offset;
                                break;
                        /*
                         * In S_FREQ state ignore updates until the stepout
                         * threshold. After that, correct the phase and
                         * frequency and switch to S_SYNC state.
                         */
                        case FREQ:
                                if (c.t - s.t < WATCH)
                                        return (IGNORE);
                                freq = (offset - c.base - c.offset) / mu;
                                break;
                        /*
                         * We get here by default in S_SYNC and S_SPIK states.
                         * Here we compute the frequency update due to PLL and
                         * FLL contributions.
                         */
                        default:
                                /*
                                 * The FLL and PLL frequency gain constants
                                 * depend on the poll interval and Allan
                                 * intercept. The FLL is not used below one-half
                                 * the Allan intercept. Above that the loop gain
                                 * increases in steps to 1 / AVG.
                                 */
                                if (LOG2D(s.poll) > ALLAN / 2)
                                {
                                        etemp = FLL - s.poll;
                                        if (etemp < AVG)
                                                etemp = AVG;
                                        freq += (offset - c.offset) / (max(mu,
                                                                           ALLAN) *
                                                                       etemp);
                                }
                                /*
                                 * For the PLL the integration interval
                                 * (numerator) is the minimum of the update
                                 * interval and poll interval. This allows
                                 * oversampling, but not undersampling.
                                 */
                                etemp = min(mu, LOG2D(s.poll));
                                dtemp = 4 * PLL * LOG2D(s.poll);
                                freq += offset * etemp / (dtemp * dtemp);
                                break;
                        }
                        rstclock(SYNC, p->t, offset);
                }
                /*
                 * Calculate the new frequency and frequency stability (wander).
                 * Compute the clock wander as the RMS of exponentially weighted
                 * frequency differences. This is not used directly, but can,
                 * along withthe jitter, be a highly useful monitoring and
                 * debugging tool
                 */
                freq += c.freq;
                c.freq = max(min(MAXFREQ, freq), -MAXFREQ);
                etemp = SQUARE(c.wander);
                dtemp = SQUARE(freq);
                c.wander = SQRT(etemp + (dtemp - etemp) / AVG);
                /*
                 * Here we adjust the poll interval by comparing the current
                 * offset with the clock jitter. If the offset is less than the
                 * * clock jitter times a constant, then the averaging interval is
                 * increased, otherwise it is decreased. A bit of hysteresis
                 * helps calm the dance. Works best using burst mode.
                 */
                if (fabs(c.offset) < PGATE * c.jitter)
                {
                        c.count += s.poll;
                        if (c.count > LIMIT)
                        {
                                c.count = LIMIT;
                                if (s.poll < MAXPOLL)
                                {
                                        c.count = 0;
                                        s.poll++;
                                }
                        }
                }
                else
                {
                        c.count -= s.poll << 1;
                        if (c.count < -LIMIT)
                        {
                                c.count = -LIMIT;
                                if (s.poll > MINPOLL)
                                {
                                        c.count = 0;
                                        s.poll--;
                                }
                        }
                }
                return (rval);
        }

        // 6.7 rstclock()

        /*
         * rstclock() - clock state machine
         */
        void
        rstclock(
            int state,     /* new state */
            double offset, /* new offset */
            double t       /* new update time */
        );
        {
                /*
                 * Enter new state and set state variables. Note we use the time
                 * of the last clock filter sample, which must be earlier than
                 * the current time.
                 */
                c.state = state;
                c.base = offset - c.offset;
                c.last = c.offset = offset;
                s.t = t;
        }

        // 7. Clock Adjust Process

        // 7.1 clock_adjust()

        /*
         * clock_adjust() - runs at one-second intervals
         */
        void
        clock_adjust();
        {
                double dtemp;
                /*
                 * Update the process time c.t. Also increase the dispersion
                 * since the last update. In contrast to NTPv3, NTPv4 does not
                 * declare unsynchronized after one day, since the dispersion
                 * threshold serves this function. When the dispersion exceeds
                 * MAXDIST (1 s), the server is considered unaccept for
                 * synchroniztion.
                 */
                c.t++;
                s.rootdisp += PHI;
                /*
                 * Implement the phase and frequency adjustments. The gain
                 * factor (denominator) is not allowed to increase beyond the
                 * Allan intercept. It doesn’t make sense to average phase noise
                 * beyond this point and it helps to damp residual offset at the
                 * longer poll intervals.
                 */
                dtemp = c.offset / (PLL * min(LOG2D(s.poll), ALLAN));
                c.offset -= dtemp;
                /*
                 * This is the kernel adjust time function, usually implemented
                 * by the Unix adjtime() system call.
                 */
                adjust_time(c.freq + dtemp);
                /*
                 * Peer timer. Call the poll() routine when the poll timer
                 * expires.
                 */
                while (/* all associations */ 0)
                {
                        struct p *p; /* dummy peer structure pointer */
                        if (c.t >= p->next)
                                poll(p);
                }
                /*
                 * Once per hour write the clock frequency to a file
                 */
                if (c.t % 3600 == 3599)
                        /* write c.freq to file */ 0;
        }

        // 8. Poll Process

#include "ntp4.h"
/*
 * Constants
 */
#define UNREACH 12 /* unreach counter threshold */
#define BCOUNT 8   /* packets in a burst */
#define BTIME 2    /* burst interval (s) */

        // 8.1 poll()

        /*
         * poll() - determine when to send a packet for association p->
         */
        void
            poll(
                struct p * p /* peer structure pointer */
            );
        {
                int hpoll;
                int oreach;
                /*
                 * This routine is called when the current time c.t catches up
                 * to the next poll time p->next. The value p->last is
                 * the last time this routine was executed. The poll_update()
                 * routine determines the next execution time p->next.
                 *
                 * If broadcasting, just do it, but only if we are synchronized.
                 */
                hpoll = p->hpoll;
                if (p->mode == M_BCST)
                {
                        p->last = c.t;
                        if (s.p != NULL)
                                peer_xmit(p);
                        poll_update(p, hpoll);
                        return;
                }
                if (p->burst == 0)
                {
                        /*
                         * We are not in a burst. Shift the reachability
                         * register to the left. Hopefully, some time before the
                         * next poll a packet will arrive and set the rightmost
                         * bit.
                         */

                        p->last = c.t;
                        oreach = p->reach;
                        p->reach << 1;
                        if (!p->reach)
                        {
                                /*
                                 * The server is unreachable, so bump the
                                 * unreach counter. If the unreach threshold has
                                 * been reached, double the poll interval to
                                 * minimize wasted network traffic.
                                 */
                                if (p->flags & P_IBURST && p->unreach == 0)
                                {
                                        p->burst = BCOUNT;
                                }
                                else if (p->unreach < UNREACH)
                                        p->unreach++;
                                else
                                        hpoll++;
                                p->unreach++;
                        }
                        else
                        {
                                /*
                                 * The server is reachable. However, if has not
                                 * been heard for three consecutive poll
                                 * intervals, stuff the clock register to
                                 * increase the peer dispersion. This makes old
                                 * servers less desirable and eventually boots
                                 * them off the island.
                                 */
                                p->unreach = 0;
                                if (!(p->reach & 0x7))
                                        clock_filter(p, 0, 0, MAXDISP);
                                hpoll = s.poll;
                                if (p->flags & P_BURST && accept(p))
                                        p->burst = BCOUNT;
                        }
                }
                else
                {
                        /*
                         * If in a burst, count it down. When the reply comes
                         * back the clock_filter() routine will call
                         * clock_select() to process the results of the burst.
                         */
                        p->burst--;
                }
                /*
                 * Do not transmit if in broadcast client mode.
                 */
                if (p->mode != M_BCLN)
                        peer_xmit(p);
                poll_update(p, hpoll);
        }

        // 8.2 poll_update()

        /*
         * poll_update() - update the poll interval for association p
         *
         * Note: This routine is called by both the packet() and poll() routine.
         * Since the packet() routine is executed when a network packet arrives
         * and the poll() routine is executed as the result of timeout, a
         * potential race can occur, possibly causing an incorrect interval for
         * the next poll. This is considered so unlikely as to be negligible.
         */
        void poll_update(
            struct p * p, /* peer structure pointer */
            int hpoll     /* poll interval (log2 s) */
        );
        {
                int poll;
                /*
                 * This routine is called by both the poll() and packet()
                 * routines to determine the next poll time. If within a burst
                 * the poll interval is two seconds. Otherwise, it is the
                 * minimum of the host poll interval and peer poll interval, but
                 * not greater than MAXPOLL and not less than MINPOLL. The
                 * design insures that a longer interval can be preempted by a
                 * shorter one if required for rapid response.
                 */
                p->hpoll = min(MAXPOLL, max(MINPOLL, hpoll));
                if (p->burst != 0)
                {
                        if (c.t != p->next)
                                return;
                        p->next += BTIME;
                }
                else
                {
                        poll = min(p->hpoll, max(MINPOLL, ppoll));
                }
                /*
                 * While not shown here, the reference implementation
                 * randonizes the poll interval by a small factor.
                 */
                p->next = p->last + (1 << poll);
        }
        /*
         * It might happen that the due time has already passed. If so,
         * make it one second in the future.
         */
        if (p->next <= c.t)
                p->next = c.t + 1;
}

// 8.3 transmit()

/*
 * transmit() - transmit a packet for association p
 */
void peer_xmit(
    struct p *p /* peer structure pointer */
)
{
        struct x x; /* transmit packet */
        /*
         * Initialize header and transmit timestamp
         */
        x.srcaddr = p->dstaddr;
        x.dstaddr = p->srcaddr;
        x.leap = s.leap;
        x.version = VERSION;
        x.mode = p->mode;
        if (s.stratum == MAXSTRAT)
                x.stratum = 0;
        else
                x.stratum = s.stratum;
        x.poll = p->hpoll;
        x.precision = s.precision;
        x.rootdelay = D2FP(s.rootdelay);
        x.rootdisp = D2FP(s.rootdisp);
        x.refid = s.refid;
        x.reftime = s.reftime;
        x.org = p->org;
        x.rec = p->rec;
        x.xmt = get_time();
        p->xmt = x.xmt;
        /*
         * If the key ID is nonzero, send a valid MAC using the key ID
         * of the association and the key in the local key cache. If
         * something breaks, like a missing trusted key, don’t send the
         * packet; just reset the association and stop until the problem
         * is fixed.
         */
        if (p->keyid)
                if (/* p->keyid invalid */ 0)
                {
                        clear(p, X_NKEY);
                        return;
                }
        x.digest = md5(p->keyid);
        xmit_packet(&x);
}