// poll

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
void poll(
    struct p *p /* peer structure pointer */
)
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
    struct p *p, /* peer structure pointer */
    int hpoll    /* poll interval (log2 s) */
)
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
        poll = min(p->hpoll, max(MINPOLL, ppoll)); // Need to be solved. ppoll is found by received packet's ppoll variable
    }                                              // Solved with p->ppoll with correct recieved packet pointer?
    /*
     * While not shown here, the reference implementation
     * randonizes the poll interval by a small factor.
     */
    p->next = p->last + (1 << poll);
    // } I comment "}" out as I beleive it should not exist // Marcus
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