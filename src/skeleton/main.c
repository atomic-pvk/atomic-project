// main.c - main program
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