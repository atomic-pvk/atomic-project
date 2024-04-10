#include "ntp4.h"

// 6.1 clock_select()

/*
 * clock_select() - find the best clocks
 */
void clock_select()
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
    struct p *p /* peer structure pointer */
)
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
int accept(
    struct p *p /* peer structure pointer */
)
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
void clock_update(
    struct p *p /* peer structure pointer */
)
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
void clock_combine()
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
