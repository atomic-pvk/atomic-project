#include "NTP_peer.h"

// A.5.1.1.  packet()

double log2d(int a)
{
    if (a < 0)
    {
        return 1.0 / (1L << -a);
    }
    else
    {
        return (double)(1L << a);
    }
}
/*
 * Dispatch matrix
 *              active  passv  client server bcast */
int table[7][5] = {
    /* nopeer  */ {NEWPS, DSCRD, FXMIT, MANY, NEWBC},
    /* active  */ {PROC, PROC, DSCRD, DSCRD, DSCRD},
    /* passv   */ {PROC, ERR, DSCRD, DSCRD, DSCRD},
    /* client  */ {DSCRD, DSCRD, DSCRD, PROC, DSCRD},
    /* server  */ {DSCRD, DSCRD, DSCRD, DSCRD, DSCRD},
    /* bcast   */ {DSCRD, DSCRD, DSCRD, DSCRD, DSCRD},
    /* bclient */ {DSCRD, DSCRD, DSCRD, DSCRD, PROC}};
/*
 * packet() - process packet and compute offset, delay, and
 * dispersion.
 */
void packet(struct ntp_p *p, /* peer structure pointer */
            struct ntp_r *r  /* receive packet pointer */
)
{
    double offset; /* sample offsset */
    double delay;  /* sample delay */
    double disp;   /* sample dispersion */

    /*
     * By golly the packet is valid.  Light up the remaining header
     * fields.  Note that we map stratum 0 (unspecified) to MAXSTRAT
     * to make stratum comparisons simpler and to provide a natural
     * interface for radio clock drivers that operate for
     *  convenience at stratum 0.
     */
    p->leap = r->leap;
    if (r->stratum == 0)
        p->stratum = MAXSTRAT;
    else
        p->stratum = r->stratum;
    p->pmode = r->mode;
    p->ppoll = r->poll;
    p->rootdisp = FP2D(r->rootdisp);
    p->refid = r->refid;
    FreeRTOS_printf_wrapper_double("disp is %d\n", r->refid);
    p->reftime = r->reftime;

    /*
     * Verify the server is synchronized with valid stratum and
     * reference time not later than the transmit time.
     */
    if (p->leap == NOSYNC || p->stratum >= MAXSTRAT)
    {
        return; /* unsynchronized */
    }

    if (r->rootdelay / 2 + r->rootdisp >= MAXDISP || p->reftime > r->xmt) return; /* invalid header values */

    poll_update(p, p->hpoll);
    p->reach |= 1;

    /*
     * Calculate offset, delay and dispersion, then pass to the
     * clock filter.  Note carefully the implied processing.  The
     * first-order difference is done directly in 64-bit arithmetic,
     * then the result is converted to floating double.  All further
     * processing is in floating-double arithmetic with rounding
     * done by the hardware.  This is necessary in order to avoid
     * overflow and preserve precision.
     *
     * The delay calculation is a special case.  In cases where the
     * server and client clocks are running at different rates and
     * with very fast networks, the delay can appear negative.  In
     * order to avoid violating the Principle of Least Astonishment,
     * the delay is clamped not less than the system precision.
     */
    if (p->pmode == M_BCST)
    {
        offset = LFP2D(subtract_uint64_t(r->xmt, r->dst));
        delay = BDELAY;
        disp = LOG2D(r->precision) + LOG2D(s.precision) + PHI * 2 * BDELAY;
    }
    else
    {
        offset = LFP2D(add_int64_t(subtract_uint64_t(r->rec, r->org), subtract_uint64_t(r->dst, r->xmt))) / 2;

        delay = max(LFP2D(subtract_uint64_t((subtract_uint64_t(r->dst, r->org)), (subtract_uint64_t(r->rec, r->xmt)))),
                    LOG2D(s.precision));
        disp = LOG2D(r->precision) + LOG2D(s.precision) + LFP2D(PHI * subtract_uint64_t(r->dst, r->org));
    }
    double local_delay = (r->dst - r->org) - (r->xmt - r->rec);
    p->rootdelay = r->rootdelay + delay;

    // FreeRTOS_printf(("\n\ntesting\n"));
    // FreeRTOS_printf_wrapper_double("", LFP2D(PHI * subtract_uint64_t(r->dst, r->org)));
    // FreeRTOS_printf_wrapper_double(
    //     "", LOG2D(r->precision) + LOG2D(s.precision) + LFP2D(PHI * subtract_uint64_t(r->dst, r->org)));
    // FreeRTOS_printf(("\n\noffset\n"));
    // FreeRTOS_printf_wrapper_double("", offset);
    // FreeRTOS_printf(("\n\ndelay\n"));
    // FreeRTOS_printf_wrapper_double("", delay);
    // FreeRTOS_printf(("\n\ndisp\n"));
    // FreeRTOS_printf_wrapper_double("", disp);
    // // FreeRTOS_printf(("\n\n\n lets see if offset is working: %d\n\n\n", offset)); // = 0
    // // FreeRTOS_printf(("\n\n\ndelay is %d\n\n\n", delay));
    // // FreeRTOS_printf(("\n\n\ndisp is %d\n\n\n", disp));
    FreeRTOS_printf(("I AM CALLING CLOCK_FILTER\n\n"));
    clock_filter(p, offset, delay, disp);
}

// A.5.2.  clock_filter()

/*
 * clock_filter(p, offset, delay, dispersion) - select the best from the
 * latest eight delay/offset samples.
 */
void clock_filter(struct ntp_p *p, /* peer structure pointer */
                  double offset,   /* clock offset */
                  double delay,    /* roundtrip delay */
                  double disp      /* dispersion */
)
{
    struct ntp_f f[NSTAGE]; /* sorted list */
    double dtemp;
    int i;

    FreeRTOS_printf(("\n\n\nCLOCK_FILTER\n\n\n"));

    /*
     * The clock filter contents consist of eight tuples (offset,
     * delay, dispersion, time).  Shift each tuple to the left,
     * discarding the leftmost one.  As each tuple is shifted,
     * increase the dispersion since the last filter update.  At the
     * same time, copy each tuple to a temporary list.  After this,
     * place the (offset, delay, disp, time) in the vacated
     * rightmost tuple.
     */
    gettime(1);  // update c.localTime to the current time
    double tmpDisp = LFP2D(
        PHI * (subtract_uint64_t(c.localTime, p->t)));  // this is a static value so it should only be calculated once

    for (i = 1; i < NSTAGE; i++)
    {
        p->f[i] = p->f[i - 1];
        p->f[i].disp += tmpDisp;
        f[i] = p->f[i];
    }
    p->f[0].t = c.localTime;
    p->f[0].offset = offset;
    p->f[0].delay = delay;
    p->f[0].disp = disp;
    f[0] = p->f[0];

    /*
     * Sort the temporary list of tuples by increasing f[].delay.
     * The first entry on the sorted list represents the best
     * sample, but it might be old.
     */
    dtemp = p->offset;
    p->offset = offset;
    p->delay = delay;

    for (i = 0; i < NSTAGE; i++)
    {
        int exp = i + 1;
        unsigned int denominator = 1 << exp;
        double result = f[i].disp / denominator;
        p->disp += result;
        p->jitter += SQUARE(f[i].offset - f[0].offset);
    }

    p->jitter = max(sqrt(p->jitter), LOG2D(s.precision));

    /*
     * Prime directive: use a sample only once and never a sample
     * older than the latest one, but anything goes before first
     * synchronized.
     */

    if (subtract_uint64_t(f[0].t, p->t) <= 0 && s.leap != NOSYNC)
    {
        FreeRTOS_printf_wrapper_double("", subtract_uint64_t(f[0].t, p->t));
        FreeRTOS_printf_wrapper_double("", f[0].t);
        FreeRTOS_printf_wrapper_double("", p->t);
        FreeRTOS_printf(("f[0].t - p->t <= 0\n"));
        return;
    }

    /*
     * Popcorn spike suppressor.  Compare the difference between the
     * last and current offsets to the current jitter.  If greater
     * than SGATE (3) and if the interval since the last offset is
     * less than twice the system poll interval, dump the spike.
     * Otherwise, and if not in a burst, shake out the truechimers.
     */
    if (fabs(p->offset - dtemp) > SGATE * p->jitter && (f[0].t - p->t) < 2 * s.poll)
    {
        FreeRTOS_printf(("Popcorn spike found\n"));
        return;
    }

    p->t = f[0].t;
    if (p->burst == 0)
    {
        FreeRTOS_printf(("p->burst == 0\n\n\n"));
        assoc_table_update(p);
        clock_select();
    }

    return;
}

/*
 * fit() - test if association p is acceptable for synchronization
 */
int fit(struct ntp_p *p /* peer structure pointer */
)
{
    /*
     * A stratum error occurs if (1) the server has never been
     * synchronized, (2) the server stratum is invalid.
     */
    if (p->leap == NOSYNC || p->stratum >= MAXSTRAT) return (FALSE);

    /*
     * A distance error occurs if the root distance exceeds the
     * distance threshold plus an increment equal to one poll
     * interval.
     */
    if (root_dist(p) > MAXDIST + PHI * (double)LOG2D(s.poll)) return (FALSE);

    /*
     * A loop error occurs if the remote peer is synchronized to the
     * local peer or the remote peer is synchronized to the current
     * system peer.  Note this is the behavior for IPv4; for IPv6
     * the MD5 hash is used instead.
     */
    if (p->refid == p->dstaddr || p->refid == s.refid) return (FALSE);

    /*
     * An unreachable error occurs if the server is unreachable.
     */
    if (p->reach == 0) return (FALSE);

    return (TRUE);
}

/*
 * clear() - reinitialize for persistent association, demobilize
 * for ephemeral association.
 */
void clear(struct ntp_p *p, /* peer structure pointer */
           int kiss         /* kiss code */
)
{
    int i;

    /*
     * The first thing to do is return all resources to the bank.
     * Typical resources are not detailed here, but they include
     * dynamically allocated structures for keys, certificates, etc.
     * If an ephemeral association and not initialization, return
     * the association memory as well.
     */
    /* return resources */
    if (s.p == p) s.p = NULL;

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
    p->disp = MAXDISPAlg;
    p->jitter = LOG2D(s.precision);
    p->refid = kiss;
    for (i = 0; i < NSTAGE; i++) p->f[i].disp = MAXDISPAlg;

    /*
     * Randomize the first poll just in case thousands of broadcast
     * clients have just been stirred up after a long absence of the
     * broadcast server.
     */
    p->outdate = p->t = c.localTime;
    p->nextdate = p->outdate + (random() & ((1 << MINPOLL) - 1));
}

// A.5.3.  fast_xmit()

/*
 * fast_xmit() - transmit a reply packet for receive packet r
 */
void fast_xmit(struct ntp_r *r, /* receive packet pointer */
               int mode,        /* association mode */
               int auth         /* authentication code */
)
{
    struct ntp_x x;

    /*
     * Initialize header and transmit timestamp.  Note that the
     * transmit version is copied from the receive version.  This is
     * for backward compatibility.
     */
    x.version = r->version;
    x.srcaddr = r->dstaddr;
    x.dstaddr = r->srcaddr;
    x.leap = s.leap;
    x.mode = mode;
    if (s.stratum == MAXSTRAT)
        x.stratum = 0;
    else
        x.stratum = s.stratum;
    x.poll = r->poll;
    x.precision = s.precision;
    x.rootdelay = D2FP(s.rootdelay);
    x.rootdisp = D2FP(s.rootdisp);
    x.refid = s.refid;
    x.reftime = s.reftime;
    x.org = r->xmt;
    x.rec = r->dst;

    gettime(0);
    x.xmt = c.localTime;

    /*
     * If the authentication code is A.NONE, include only the
     * header; if A.CRYPTO, send a crypto-NAK; if A.OK, send a valid
     * MAC.  Use the key ID in the received packet and the key in
     * the local key cache.
     */
    if (auth != A_NONE)
    {
        if (auth == A_CRYPTO)
        {
            x.keyid = 0;
        }
        else
        {
            x.keyid = r->keyid;
            x.dgst = md5(x.keyid);
        }
    }
    xmit_packet(&x);
}

// A.5.4.  access()

/*
 * access() - determine access restrictions
 */
int access(struct ntp_r *r /* receive packet pointer */
)
{
    /*
     * The access control list is an ordered set of tuples
     * consisting of an address, mask, and restrict word containing
     * defined bits.  The list is searched for the first match on
     * the source address (r->srcaddr) and the associated restrict
     * word is returned.
     */
    return (/* access bits */ 1);  // TODO ALL HAVE ACCESS AT ALL TIME CURRENTLY
}

/*
 * receive() - receive packet and decode modes
 */
void receive(struct ntp_r *r /* receive packet pointer */
             // I am adding this to the function signature to make it compile and since I do not see any other way to
             // "check" the system structure pointer
)
{
    struct ntp_p *p; /* peer structure pointer */
    int auth;        /* authentication code */
    int has_mac;     /* size of MAC */
    int synch;       /* synchronized switch */

    /*
     * Check access control lists.  The intent here is to implement
     * a whitelist of those IP addresses specifically accepted
     * and/or a blacklist of those IP addresses specifically
     * rejected.  There could be different lists for authenticated
     * clients and unauthenticated clients.
     */
    if (!access(r)) return; /* access denied */

    /*
     * The version must not be in the future.  Format checks include
     * packet length, MAC length and extension field lengths, if
     * present.
     */
    if (r->version > VERSION /* or format error */) return; /* format error */

    /*
     * Authentication is conditioned by two switches that can be
     * specified on a per-client basis.
     *
     * P_NOPEER     do not mobilize an association unless
     *              authenticated.
     * P_NOTRUST    do not allow access unless authenticated
     *              (implies P_NOPEER).
     *
     * There are four outcomes:
     *
     * A_NONE       the packet has no MAC.
     * A_OK         the packet has a MAC and authentication
     *               succeeds.
     * A_ERROR      the packet has a MAC and authentication fails.
     * A_CRYPTO     crypto-NAK.  The MAC has four octets only.
     *
     * Note: The AUTH (x, y) macro is used to filter outcomes.  If x
     * is zero, acceptable outcomes of y are NONE and OK.  If x is
     * one, the only acceptable outcome of y is OK.
     */

    has_mac = /* length of MAC field */ 0;
    if (has_mac == 0)
    {
        auth = A_NONE; /* not required */
    }
    else if (has_mac == 4)
    {
        auth = A_CRYPTO; /* crypto-NAK */
    }
    else
    {
        if (r->mac != md5(r->keyid))
            auth = A_ERROR; /* auth error */
        else
            auth = A_OK; /* auth OK */
    }

    /*
     * Find association and dispatch code.  If there is no
     * association to match, the value of p->hmode is assumed NULL.
     */
    p = find_assoc(r);
    switch (table[(unsigned int)(p->hmode)][(unsigned int)(r->mode) - 1])  // PACKET MODE IS INDEXED FROM 1
    {                                                                      // WHEN TABLE IS INDEXED FROM 0

        /*
         * Client packet and no association.  Send server reply without
         * saving state.
         */
        case FXMIT:

            /*
             * If unicast destination address, send server packet.
             * If authentication fails, send a crypto-NAK packet.
             */

            /* not multicast dstaddr */
            if (0)
            {
                if (AUTH(p->flags & P_NOTRUST, auth))
                    fast_xmit(r, M_SERV, auth);
                else if (auth == A_ERROR)
                    fast_xmit(r, M_SERV, A_CRYPTO);
                return; /* M_SERV packet sent */
            }

            /*
             * This must be manycast.  Do not respond if we are not
             * synchronized or if our stratum is above the
             * manycaster.
             */
            if (s.leap == NOSYNC || s.stratum > r->stratum) return;

            /*
             * Respond only if authentication is OK.  Note that the
             * unicast address is used, not the multicast.
             */
            if (AUTH(p->flags & P_NOTRUST, auth)) fast_xmit(r, M_SERV, auth);
            return;

        /*
         * New manycast client ephemeral association.  It is mobilized
         * in the same version as in the packet.  If authentication
         * fails, ignore the packet.  Verify the server packet by
         * comparing the r->org timestamp in the packet with the p->xmt
         * timestamp in the multicast client association.  If they
         * match, the server packet is authentic.  Details omitted.
         */
        case MANY:
            if (!AUTH(p->flags & (P_NOTRUST | P_NOPEER), auth)) return; /* authentication error */

            p = mobilize(r->srcaddr, r->dstaddr, r->version, M_CLNT, r->keyid, P_EPHEM);
            break;

            /*
             * New symmetric passive association.  It is mobilized in the
             * same version as in the packet.  If authentication fails,
             * send a crypto-NAK packet.  If restrict no-moblize, send a
             * symmetric active packet instead.
             */
        case NEWPS:
            if (!AUTH(p->flags & P_NOTRUST, auth))
            {
                if (auth == A_ERROR) fast_xmit(r, M_SACT, A_CRYPTO);
                return; /* crypto-NAK packet sent */
            }
            if (!AUTH(p->flags & P_NOPEER, auth))
            {
                fast_xmit(r, M_SACT, auth);
                return; /* M_SACT packet sent */
            }
            p = mobilize(r->srcaddr, r->dstaddr, r->version, M_PASV, r->keyid, P_EPHEM);
            break;

        /*
         * New broadcast client association.  It is mobilized in the
         * same version as in the packet.  If authentication fails,
         * ignore the packet.  Note this code does not support the
         * initial volley feature in the reference implementation.
         */
        case NEWBC:
            if (!AUTH(p->flags & (P_NOTRUST | P_NOPEER), auth)) return; /* authentication error */

            if (!(s.flags & S_BCSTENAB)) return; /* broadcast not enabled */

            p = mobilize(r->srcaddr, r->dstaddr, r->version, M_BCLN, r->keyid, P_EPHEM);
            break; /* processing continues *

/*
* Process packet.  Placeholdler only.
*/
        case PROC:
            // p = mobilize(r->srcaddr, r->dstaddr, r->version, M_SERV,
            //              r->keyid, P_EPHEM); // TODO //
            break; /* processing continues */

        /*
         * Invalid mode combination.  We get here only in case of
         * ephemeral associations, so the correct action is simply to
         * toss it.
         */
        case ERR:
            clear(p, X_ERROR);
            return; /* invalid mode combination */

        /*
         * No match; just discard the packet.
         */
        case DSCRD:
            return; /* orphan abandoned */
    }
    /*
     * Next comes a rigorous schedule of timestamp checking.  If the
     * transmit timestamp is zero, the server is horribly broken.
     */
    time_t rXmtInSeconds = (time_t)((r->xmt >> 32) - 2208988800ull);
    uint32_t rXmtFrac = (uint32_t)(r->xmt & 0xFFFFFFFF);

    if (rXmtInSeconds == 0 && rXmtFrac == 0) return; /* invalid timestamp */

    /*
     * If the transmit timestamp duplicates a previous one, the
     * packet is a replay.
     */
    time_t pXmtInSeconds = (time_t)((p->xmt >> 32) - 2208988800ull);
    uint32_t pXmtFrac = (uint32_t)(p->xmt & 0xFFFFFFFF);

    if ((rXmtInSeconds == pXmtInSeconds) && (rXmtFrac == pXmtFrac))
    {
        return; /* duplicate packet */
    }

    /*
     * If this is a broadcast mode packet, skip further checking.
     * If the origin timestamp is zero, the sender has not yet heard
     * from us.  Otherwise, if the origin timestamp does not match
     * the transmit timestamp, the packet is bogus.
     */
    time_t rOrgInSeconds = (time_t)((r->org >> 32) - 2208988800ull);
    uint32_t rOrgFrac = (uint32_t)(r->org & 0xFFFFFFFF);
    synch = TRUE;
    if (r->mode != M_BCST)
    {
        if ((rOrgInSeconds == 0) && (rOrgFrac == 0))
        {
            FreeRTOS_printf(("rOrgInSeconds == 0\n"));
            synch = FALSE; /* unsynchronized */
        }

        else if ((rOrgInSeconds != pXmtInSeconds) && (rOrgFrac != pXmtFrac))
        {
            FreeRTOS_printf(("rOrgInSeconds != pXmtInSeconds\n"));
            synch = FALSE; /* bogus packet */
        }
    }
    else
    {
        FreeRTOS_printf(("r->mode == M_BCST\n"));
    }

    /*
     * Update the origin and destination timestamps.  If
     * unsynchronized or bogus, abandon ship.
     */
    p->org = r->xmt;
    p->rec = r->dst;
    if (!synch) return; /* unsynch */

    /*
     * The timestamps are valid and the receive packet matches the
     * last one sent.  If the packet is a crypto-NAK, the server
     * might have just changed keys.  We demobilize the association
     * and wait for better times.
     */
    if (auth == A_CRYPTO)
    {
        clear(p, X_CRYPTO);
        return; /* crypto-NAK */
    }

    /*
     * If the association is authenticated, the key ID is nonzero
     * and received packets must be authenticated.  This is designed
     * to avoid a bait-and-switch attack, which was possible in past
     * versions.
     */
    if (!AUTH(p->keyid || (p->flags & P_NOTRUST), auth)) return; /* bad auth */

    /*
     * Everything possible has been done to validate the timestamps
     * and prevent bad guys from disrupting the protocol or
     * injecting bogus data.  Earn some revenue.
     */
    FreeRTOS_printf(("calling packet\n"));
    packet(p, r);
}

// double sqrt(double number) {
//     double error = 0.00001;  // Define the precision of your answer
//     double s = number;

//     if(number == 0) return 0.0;

//     while ((s - number / s) > error || (number / s - s) > error) {
//         s = (s + number / s) / 2;
//     }

//     return s;
// }