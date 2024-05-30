#include "NTP_system_process.h"

#include "NTPTask.h"
#include "NTP_peer.h"
#include "NTP_vfo.h"

/**
 * This function is a comparator for the qsort function. It compares two ntp_m structures.
 *
 * @param a - The first void pointer that will be cast to an ntp_m structure.
 * @param b - The second void pointer that will be cast to an ntp_m structure.
 *
 * @return - It returns -1 if the edge of the first structure is less than the edge of the second,
 *           1 if the edge of the first structure is greater than the edge of the second,
 *           and 0 if the edges of both structures are equal.
 */
int compare_ntp_m(const void *a, const void *b)
{
    struct ntp_m *elem1 = (struct ntp_m *)a;
    struct ntp_m *elem2 = (struct ntp_m *)b;

    if (elem1->edge < elem2->edge) return -1;
    if (elem1->edge > elem2->edge) return 1;
    return 0;
}

/**
 * This function is used to cull the falsetickers from the server population, leaving only the truechimers.
 * The correctness interval for association p is the interval from offset - root_dist() to offset + root_dist().
 * The function constructs a chime list of tuples (p, type, edge) and sorts the list by edge from lowest to highest.
 *
 * @param n - The pointer to an integer that will be incremented by 3 for each peer that passes the fit function.
 */
void cull(int *n)
{
    int i = 0;
    struct ntp_p *p;

    FreeRTOS_printf(("Culling falsechimes\n"));

    for (int index = 0; index < NMAX; index++)
    {
        p = assoc_table->peers[index];
        if (fit(p))
        {
            s.m[i].p = p;
            s.m[i].type = -1;
            s.m[i].edge = p->offset - root_dist(p);
            i++;

            s.m[i].p = p;
            s.m[i].type = 0;
            s.m[i].edge = p->offset;
            i++;

            s.m[i].p = p;
            s.m[i].type = +1;
            s.m[i].edge = p->offset + root_dist(p);
            i++;
        }
    }

    qsort(&s.m, i, sizeof(struct ntp_m), compare_ntp_m);

    FreeRTOS_printf(("Truechimes added\n"));
    for (int index = 0; index < i; index++)
    {
        FreeRTOS_printf_wrapper_double("\n\n%s\n\n", s.m[index].edge);
    }
    *n = i;
}

/**
 * Finds the largest contiguous intersection of correctness intervals.
 *
 * @param n The number of correctness intervals.
 * @param low Pointer to store the lower endpoint of the intersection.
 * @param high Pointer to store the higher endpoint of the intersection.
 */
void intersection(int n, double *low, double *high)
{
    /*
     * Find the largest contiguous intersection of correctness
     * intervals.  Allow is the number of allowed falsetickers;
     * found is the number of midpoints.  Note that the edge values
     * are limited to the range +-(2 ^ 30) < +-2e9 by the timestamp
     * calculations.
     */
    FreeRTOS_printf(("Finding largest contiguous intersection of correctness intervals \n"));
    int i, allow, found, chime; /* used by intersection algorithm */
    *low = 2e9;
    *high = -2e9;
    for (allow = 0; 2 * allow < n; allow++)
    {
        /*
         * Scan the chime list from lowest to highest to find
         * the lower endpoint.
         */
        chime = 0;
        int lastchime = 0;

        for (i = 0; i < n; i++)
        {
            chime -= s.m[i].type;
            if (chime < lastchime)
            {
                *high = s.m[i].edge;
                break;
            }
            lastchime = chime;
        }

        /*
         * Scan the chime list from highest to lowest to find
         * the lower endpoint.
         */
        chime = 0;
        lastchime = 0;
        for (i = 0; i < n; i++)
        {
            chime += s.m[i].type;
            if (chime == lastchime)
            {
                *low = s.m[i - 1].edge;
                break;
            }
            lastchime = chime;
        }
        /*
         * If the number of midpoints is greater than the number
         * of allowed falsetickers, the intersection contains at
         * least one truechimer with no midpoint.  If so,
         * increment the number of allowed falsetickers and go
         * around again.  If not and the intersection is
         * non-empty, declare success.
         */
        if (found > allow) continue;

        if (*high > *low) break;
    }
}

/**
 * Selects the best clock from the list of survivors based on the clustering algorithm.
 * The survivors are selected based on their correctness interval extents and clustering metrics.
 * At least NSANE survivors are required to satisfy the correctness assertions.
 * For each association, the selection jitter is calculated as the square root of the sum of squares
 * of the offset differences between the associations.
 * The function continues to discard the survivor with maximum selection jitter until a termination condition is met.
 * Finally, the best clock is picked based on the old system peer and the first survivor on the list.
 */
void clock_select()
{
    struct ntp_p *p, *osys; /* peer structure pointers */
    double low, high;       /* correctness interval extents */
    int n, i, j;

    osys = s.p;
    s.p = NULL;
    n = 0;
    FreeRTOS_printf(("Perform selection algorithm\n"));

    cull(&n);

    intersection(n, &low, &high);

    /*
     * Clustering algorithm.  Construct a list of survivors (p,
     * metric) from the chime list, where metric is dominated first
     * by stratum and then by root distance.  All other things being
     * equal, this is the order of preference.
     */
    FreeRTOS_printf(("Clustering algorithm\n"));
    s.n = 0;
    for (i = 0; i < n; i++)
    {
        // FreeRTOS_printf(("\nedge | high | low\n"));
        // FreeRTOS_printf_wrapper_double("", s.m[i].edge);
        // FreeRTOS_printf_wrapper_double("", high);
        // FreeRTOS_printf_wrapper_double("", low);
        if (s.m[i].edge < low || s.m[i].edge > high) continue;

        p = s.m[i].p;
        s.v[s.n].p = p;
        s.v[s.n].metric = (double)(MAXDIST * p->stratum) + root_dist(p);
        s.n++;
        FreeRTOS_printf(("survivor found\n"));
        FreeRTOS_printf_wrapper_double("", s.m[i].edge);
    }

    /*
     * There must be at least NSANE survivors to satisfy the
     * correctness assertions.  Ordinarily, the Byzantine criteria
     * require four survivors, but for the demonstration here, one
     * is acceptable.
     */
    if (s.n < NSANE)
    {
        FreeRTOS_printf(("survivors dead\n"));
        return;
    }

    /*
     * For each association p in turn, calculate the selection
     * jitter p->sjitter as the square root of the sum of squares
     * (p->offset - q->offset) over all q associations.  The idea is
     * to repeatedly discard the survivor with maximum selection
     * jitter until a termination condition is met.
     */
    while (1)
    {
        struct ntp_p *p, *q, *qmax; /* peer structure pointers */
        double max, min, dtemp;

        max = -2e9;
        min = 2e9;
        for (i = 0; i < s.n; i++)
        {
            p = s.v[i].p;
            if (p->jitter < min) min = p->jitter;
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
         * as well stop.  To make sure a few survivors are left
         * for the clustering algorithm to chew on, we also stop
         * if the number of survivors is less than or equal to
         * NMIN (3).
         */
        if (max < min || n <= NMIN) break;

        /*
         * Delete survivor qmax from the list and go around
         * again.
         */
        s.n--;
    }

    /*
     * Pick the best clock.  If the old system peer is on the list
     * and at the same stratum as the first survivor on the list,
     * then don't do a clock hop.  Otherwise, select the first
     * survivor on the list as the new system peer.
     */
    if (osys->stratum == s.v[0].p->stratum)
    {
        FreeRTOS_printf(("s.p = osys\n"));
        s.p = osys;
    }
    else
    {
        FreeRTOS_printf(("s.p = s.v[0].p\n"));
        s.p = s.v[0].p;
    }

    // memset(s.m, 0, sizeof(s.m));  // Clear temporary response data

    clock_update(s.p);
}

/**
 * This function calculates the root synchronization distance for a given peer.
 * The root synchronization distance is the maximum error due to all causes of the local clock relative to the primary
 * server.
 *
 * @param p - A pointer to the peer structure for which the root synchronization distance is to be calculated.
 *
 * @return - The function returns the root synchronization distance, which is calculated as half the total delay plus
 * total dispersion plus peer jitter. The total delay is the maximum of MINDISP and the sum of the root delay and delay
 * for the peer. The total dispersion is the sum of the root dispersion and dispersion for the peer. The peer jitter is
 * added as is. Finally, a term proportional to the product of the constant PHI and the difference between the current
 * time and the peer's time is added.
 */
double root_dist(struct ntp_p *p /* peer structure pointer */
)
{
    /*
     * The root synchronization distance is the maximum error due to
     * all causes of the local clock relative to the primary server.
     * It is defined as half the total delay plus total dispersion
     * plus peer jitter.
     */

    return (max(MINDISP, p->rootdelay + p->delay) / 2 + p->rootdisp + p->disp +
            PHI * LFP2D(subtract_uint64_t(c.t, p->t)) + p->jitter);
}

/*
 * accept() - test if association p is acceptable for synchronization
 */
int accept(struct ntp_p *p /* peer structure pointer */
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
    if (root_dist(p) > MAXDIST + PHI * LOG2D(s.poll)) return (FALSE);

    /*
     * A loop error occurs if the remote peer is synchronized to the
     * local peer or the remote peer is synchronized to the current
     * system peer.  Note this is the behavior for IPv4; for IPv6
     * the MD5 hash is used instead.
     *
     * Added a bypass for the second condition, as the previous
     * implementation would not allow the system to synchronize with
     * multiple different stratum 1 server using the same underlying timekeeping
     * mechanism. - A
     */

    // Adjusted condition for stratum 1 servers
    if (p->stratum != 1 && (p->refid == p->dstaddr || p->refid == s.refid)) return (FALSE);

    /*
     * An unreachable error occurs if the server is unreachable.
     */
    if (p->reach == 0) return (FALSE);

    return (TRUE);
}

/*
 * clock_update() - update the system clock
 */
void clock_update(struct ntp_p *p /* peer structure pointer */
)
{
    double dtemp;

    /*
     * If this is an old update, for instance, as the result of a
     * system peer change, avoid it.  We never use an old sample or
     * the same sample twice.
     */
    if (s.t > p->t)
    {
        FreeRTOS_printf_wrapper_double("s.t: ", s.t);
        FreeRTOS_printf_wrapper_double("p->t: ", p->t);
        FreeRTOS_printf(("s.t > p->t, discarding packet\n"));
        return;
    }
    /*
     * Combine the survivor offsets and update the system clock; the
     * local_clock() routine will tell us the good or bad news.
     */
    s.t = p->t;
    clock_combine();

    // FreeRTOS_printf(("calling local\n"));
    // FreeRTOS_printf_wrapper_double("", s.offset);
    switch (local_clock(p, s.offset))
    {
        /*
         * The offset is too large and probably bogus.  Complain to the
         * system log and order the operator to set the clock manually
         * within PANIC range.  The reference implementation includes a
         * command line option to disable this check and to change the
         * panic threshold from the default 1000 s as required.
         */
        case PANIC:
            exit(0);

        /*
         * The offset is more than the step threshold (0.125 s by
         * default).  After a step, all associations now have
         * inconsistent time values, so they are reset and started
         * fresh.  The step threshold can be changed in the reference
         * implementation in order to lessen the chance the clock might
         * be stepped backwards.  However, there may be serious
         * consequences, as noted in the white papers at the NTP project
         * site.
         */
        case STEP:
            while (/* all associations */ 0) clear(p, X_STEP);
            s.stratum = MAXSTRAT;
            s.poll = MINPOLL;
            break;

        /*
         * The offset was less than the step threshold, which is the
         * normal case.  Update the system variables from the peer
         * variables.  The lower clamp on the dispersion increase is to
         * avoid timing loops and clockhopping when highly precise
         * sources are in play.  The clamp can be changed from the
         * default .01 s in the reference implementation.
         */
        case SLEW:
            FreeRTOS_printf(("Set new time\n"));
            s.leap = p->leap;
            s.stratum = p->stratum + 1;
            s.refid = p->refid;
            s.reftime = p->reftime;
            s.rootdelay = p->rootdelay + p->delay;
            dtemp = SQRT(SQUARE(p->jitter) + SQUARE(s.jitter));
            dtemp += max(p->disp + PHI * LFP2D(subtract_uint64_t(c.t, p->t)) + fabs(p->offset), MINDISP);
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

/**
 * This function combines the offsets of the survivors from the clustering algorithm using a weighted average.
 * The weight is determined by the root distance. It also computes the selection jitter as the weighted RMS difference
 * between the first survivor and the remaining survivors.
 */
void clock_combine()
{
    struct ntp_p *p; /* peer structure pointer */
    double x, y, z, w;
    int i;
    // FreeRTOS_printf(("\nCLOCK COMBINE \n\n"));

    /*
     * Combine the offsets of the clustering algorithm survivors
     * using a weighted average with weight determined by the root
     * distance.  Compute the selection jitter as the weighted RMS
     * difference between the first survivor and the remaining
     * survivors.  In some cases, the inherent clock jitter can be
     * reduced by not using this algorithm, especially when frequent
     * clockhopping is involved.  The reference implementation can
     * be configured to avoid this algorithm by designating a
     * preferred peer.
     */
    y = z = w = 0;
    for (i = 0; s.v[i].p != NULL; i++)
    {
        p = s.v[i].p;
        x = root_dist(p);
        y += 1 / x;
        z += p->offset / x;
        w += SQUARE(fabs(p->offset - s.v[0].p->offset)) / x;
    }

    // FreeRTOS_printf(("combine: p->offset z: \n"));
    // FreeRTOS_printf_wrapper_double("combine: p->offset: ", z);

    // FreeRTOS_printf(("combine: p->offset y: \n"));
    // FreeRTOS_printf_wrapper_double("combine: p->offset: ", y);

    // FreeRTOS_printf(("combine: p->offset z / y: \n"));
    // FreeRTOS_printf_wrapper_double("combine: p->offset: ", z / y);
    s.offset = z / y;
    s.jitter = SQRT(w / y);
}