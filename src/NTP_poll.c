
#include "NTP_poll.h"

#include "NTPTask.h"
#include "NTP_main_utility.h"
#include "NTP_peer.h"
// A.5.7.1.  poll()
/*
 * poll() - determine when to send a packet for association p->
 */
void poll(struct ntp_p *p)  // peer structure pointer
{
    int hpoll;
    int oreach;

    /*
     * This routine is called when the current time c.t catches up
     * to the next poll time p->nextdate.  The value p->outdate is
     * the last time this routine was executed.  The poll_update()
     * routine determines the next execution time p->nextdate.
     *
     * If broadcasting, just do it, but only if we are synchronized.
     */
    hpoll = p->hpoll;
    if (p->hmode == M_BCST)
    {
        p->outdate = c.t;
        if (s.p != NULL) peer_xmit(p);
        poll_update(p, hpoll);
        return;
    }

    /*
     * If manycasting, start with ttl = 1.  The ttl is increased by
     * one for each poll until MAXCLOCK servers have been found or
     * ttl reaches TTLMAX.  If reaching MAXCLOCK, stop polling until
     * the number of servers falls below MINCLOCK, then start all
     * over.
     */
    if (p->hmode == M_CLNT && p->flags & P_MANY)
    {
        p->outdate = c.t;
        if (p->unreach > BEACON)
        {
            p->unreach = 0;
            p->ttl = 1;
            peer_xmit(p);
        }
        else if (s.n < MINCLOCK)
        {
            if (p->ttl < TTLMAX) p->ttl++;
            peer_xmit(p);
        }
        p->unreach++;
        poll_update(p, hpoll);
        return;
    }
    if (p->burst == 0)
    {
        /*
         * We are not in a burst.  Shift the reachability
         * register to the left.  Hopefully, some time before
         * the next poll a packet will arrive and set the
         * rightmost bit.
         */
        oreach = p->reach;
        p->outdate = c.t;
        p->reach = p->reach << 1;
        if (!(p->reach & 0x7)) clock_filter(p, 0, 0, MAXDISP);
        if (!p->reach)
        {
            /*
             * The server is unreachable, so bump the
             * unreach counter.  If the unreach threshold
             * has been reached, double the poll interval
             * to minimize wasted network traffic.  Send a
             * burst only if enabled and the unreach
             * threshold has not been reached.
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
             * The server is reachable.  Set the poll
             * interval to the system poll interval.  Send a
             * burst only if enabled and the peer is fit.
             */
            p->unreach = 0;
            hpoll = s.poll;
            if (p->flags & P_BURST && fit(p)) p->burst = BCOUNT;
        }
    }
    else
    {
        /*
         * If in a burst, count it down.  When the reply comes
         * back the clock_filter() routine will call
         * clock_select() to process the results of the burst.
         */
        p->burst--;
    }
    /*
     * Do not transmit if in broadcast client mode.
     */
    if (p->hmode != M_BCLN) peer_xmit(p);
    poll_update(p, hpoll);
}

// A.5.7.2.  poll_update()

/*
 * poll_update() - update the poll interval for association p
 *
 * Note: This routine is called by both the packet() and poll() routine.
 * Since the packet() routine is executed when a network packet arrives
 * and the poll() routine is executed as the result of timeout, a
 * potential race can occur, possibly causing an incorrect interval for
 * the next poll.  This is considered so unlikely as to be negligible.
 */
void poll_update(struct ntp_p *p, /* peer structure pointer */
                 int poll)        /* poll interval (log2 s) */
{
    /*
     * This routine is called by both the poll() and packet()
     * routines to determine the next poll time.  If within a burst
     * the poll interval is two seconds.  Otherwise, it is the
     * minimum of the host poll interval and peer poll interval, but
     * not greater than MAXPOLL and not less than MINPOLL.  The
     * design ensures that a longer interval can be preempted by a
     * shorter one if required for rapid response.
     */
    p->hpoll = max(min(MAXPOLL, poll), MINPOLL);
    if (p->burst > 0)
    {
        if (p->nextdate != c.t)
            return;
        else
            p->nextdate += BTIME;
    }
    else
    {
        /*
         * While not shown here, the reference implementation
         * randomizes the poll interval by a small factor.
         */
        p->nextdate = p->outdate + (1 << max(min(p->ppoll, p->hpoll), MINPOLL));
    }

    /*
     * It might happen that the due time has already passed.  If so,
     * make it one second in the future.
     */
    if (p->nextdate <= c.t) p->nextdate = c.t + 1;
}

// A.5.7.3.  peer_xmit()

/*
 * transmit() - transmit a packet for association p
 */
void peer_xmit(struct ntp_p *p  // peer structure pointer)
)
{
    struct ntp_x x; /* transmit packet */

    /*
     * Initialize header and transmit timestamp
     */
    x.srcaddr = p->dstaddr;
    x.dstaddr = p->srcaddr;
    x.leap = s.leap;
    x.version = p->version;
    x.mode = p->hmode;
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
    // gettime(0);
    x.xmt = c.t;
    p->xmt = x.xmt;

    /*
     * If the key ID is nonzero, send a valid MAC using the key ID
     * of the association and the key in the local key cache.  If
     * something breaks, like a missing trusted key, don't send the
     * packet; just reset the association and stop until the problem
     * is fixed.
     */
    if (p->keyid)
        if (/* p->keyid invalid */ 0)
        {
            clear(p, X_NKEY);
            return;
        }
    x.dgst = md5(p->keyid);
    xmit_packet(&x);
}