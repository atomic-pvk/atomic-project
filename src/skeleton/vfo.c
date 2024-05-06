#include "ntp4.h"
// A.5.5.6.  local_clock()

/*
 * Clock discipline parameters and constants
 */
#define STEPT .128      /* step threshold (s) */
#define WATCH 900       /* stepout threshold (s) */
#define PANICT 1000     /* panic threshold (s) */
#define PLL 65536       /* PLL loop gain */
#define FLL MAXPOLL + 1 /* FLL loop gain */
#define AVG 4           /* parameter averaging constant */
#define ALLAN 1500      /* compromise Allan intercept (s) */
#define LIMIT 30        /* poll-adjust threshold */
#define MAXFREQ 500e-6  /* frequency tolerance (500 ppm) */
#define PGATE 4         /* poll-adjust gate */

/*
 * local_clock() - discipline the local clock
 */
int                       /* return code */
local_clock(struct p *p,  /* peer structure pointer */
            double offset /* clock offset from combine() */
)
{
    int state;   /* clock discipline state */
    double freq; /* frequency */
    double mu;   /* interval since last update */
    int rval;
    double etemp, dtemp;

    /*
     * If the offset is too large, give up and go home.
     */
    if (fabs(offset) > PANICT) return (PANIC);

    /*
     * Clock state machine transition function.  This is where the
     * action is and defines how the system reacts to large time
     * and frequency errors.  There are two main regimes: when the
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
             * In S_SYNC state, we ignore the first outlier and
             * switch to S_SPIK state.
             */
            case SYNC:
                state = SPIK;
                return (rval);

            /*
             * In S_FREQ state, we ignore outliers and inliers.  At
             * the first outlier after the stepout threshold,
             * compute the apparent frequency correction and step
             * the time.
             */
            case FREQ:
                if (mu < WATCH) return (IGNORE);

                freq = (offset - c.offset) / mu;
                /* fall through to S_SPIK */

            /*
             * In S_SPIK state, we ignore succeeding outliers until
             * either an inlier is found or the stepout threshold is
             * exceeded.
             */
            case SPIK:
                if (mu < WATCH) return (IGNORE);

                /* fall through to default */

            /*
             * We get here by default in S_NSET and S_FSET states
             * and from above in S_FREQ state.  Step the time and
             * clamp down the poll interval.
             *
             * In S_NSET state, an initial frequency correction is
             * not available, usually because the frequency file has
             * not yet been written.  Since the time is outside the
             * capture range, the clock is stepped.  The frequency
             * will be set directly following the stepout interval.
             *
             * In S_FSET state, the initial frequency has been set
             * from the frequency file.  Since the time is outside
             * the capture range, the clock is stepped immediately,
             * rather than after the stepout interval.  Guys get
             * nervous if it takes 17 minutes to set the clock for
             * the first time.
             *
             * In S_SPIK state, the stepout threshold has expired
             * and the phase is still above the step threshold.
             * Note that a single spike greater than the step
             * threshold is always suppressed, even at the longer
             * poll intervals.
             */
            default:

                /*
                 * This is the kernel set time function, usually
                 * implemented by the Unix settimeofday() system
                 * call.
                 */
                step_time(offset);
                c.count = 0;
                s.poll = MINPOLL;
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
         * weighted offset differences.  This is used by the
         * poll-adjust code.
         */
        etemp = SQUARE(c.jitter);
        dtemp = SQUARE(max(fabs(offset - c.last), LOG2D(s.precision)));
        c.jitter = SQRT(etemp + (dtemp - etemp) / AVG);
        switch (c.state)
        {
            /*
             * In S_NSET state, this is the first update received
             * and the frequency has not been initialized.  The
             * first thing to do is directly measure the oscillator
             * frequency.
             */
            case NSET:
                rstclock(FREQ, p->t, offset);
                return (IGNORE);
            /*
             * In S_FSET state, this is the first update and the
             * frequency has been initialized.  Adjust the phase,
             * but don't adjust the frequency until the next update.
             */
            case FSET:
                rstclock(SYNC, p->t, offset);
                break;

            /*
             * In S_FREQ state, ignore updates until the stepout
             * threshold.  After that, correct the phase and
             * frequency and switch to S_SYNC state.
             */
            case FREQ:
                if (c.t - s.t < WATCH) return (IGNORE);

                freq = (offset - c.offset) / mu;
                break;

            /*
             * We get here by default in S_SYNC and S_SPIK states.
             * Here we compute the frequency update due to PLL and
             * FLL contributions.
             */
            default:

                /*
                 * The FLL and PLL frequency gain constants
                 * depending on the poll interval and Allan
                 * intercept.  The FLL is not used below one
                 * half the Allan intercept.  Above that the
                 * loop gain increases in steps to 1 / AVG.
                 */
                if (LOG2D(s.poll) > ALLAN / 2)
                {
                    etemp = FLL - s.poll;
                    if (etemp < AVG) etemp = AVG;
                    freq += (offset - c.offset) / (max(mu, ALLAN) * etemp);
                }

                /*
                 * For the PLL the integration interval
                 * (numerator) is the minimum of the update
                 * interval and poll interval.  This allows
                 * oversampling, but not undersampling.
                 */
                etemp = min(mu, LOG2D(s.poll));
                dtemp = 4 * PLL * LOG2D(s.poll);
                freq += offset * etemp / (dtemp * dtemp);
                rstclock(SYNC, p->t, offset);
                break;
        }
    }

    /*
     * Calculate the new frequency and frequency stability (wander).
     * Compute the clock wander as the RMS of exponentially weighted
     * frequency differences.  This is not used directly, but can,
     * along with the jitter, be a highly useful monitoring and
     * debugging tool.
     */
    freq += c.freq;
    c.freq = max(min(MAXFREQ, freq), -MAXFREQ);
    etemp = SQUARE(c.wander);
    dtemp = SQUARE(freq);
    c.wander = SQRT(etemp + (dtemp - etemp) / AVG);

    /*
     * Here we adjust the poll interval by comparing the current
     * offset with the clock jitter.  If the offset is less than the
     * clock jitter times a constant, then the averaging interval is
     * increased; otherwise, it is decreased.  A bit of hysteresis
     * helps calm the dance.  Works best using burst mode.
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

// A.5.5.7.  rstclock()

/*
 * rstclock() - clock state machine
 */
void rstclock(int state,     /* new state */
              double offset, /* new offset */
              double t       /* new update time */
)
{
    /*
     * Enter new state and set state variables.  Note, we use the
     * time of the last clock filter sample, which must be earlier
     * than the current time.
     */
    c.state = state;
    c.last = c.offset = offset;
    s.t = t;
}

// A.5.6.  Clock Adjust Process

// A.5.6.1.  clock_adjust()

/*
 * clock_adjust() - runs at one-second intervals
 */
void clock_adjust()
{
    double dtemp;

    /*
     * Update the process time c.t.  Also increase the dispersion
     * since the last update.  In contrast to NTPv3, NTPv4 does not
     * declare unsynchronized after one day, since the dispersion
     * threshold serves this function.  When the dispersion exceeds
     * MAXDIST (1 s), the server is considered unfit for
     * synchronization.
     */
    c.t++;
    s.rootdisp += PHI;

    /*
     * Implement the phase and frequency adjustments.  The gain
     * factor (denominator) is not allowed to increase beyond the
     * Allan intercept.  It doesn't make sense to average phase
     * noise beyond this point and it helps to damp residual offset
     * at the longer poll intervals.
     */
    dtemp = c.offset / (PLL * min(LOG2D(s.poll), ALLAN));
    c.offset -= dtemp;

    /*
     * This is the kernel adjust time function, usually implemented
     * by the Unix adjtime() system call.
     */
    adjust_time(c.freq + dtemp);

    /*
     * Peer timer.  Call the poll() routine when the poll timer
     * expires.
     */
    while (/* all associations */ 0)
    {
        struct p *p; /* dummy peer structure pointer */

        if (c.t >= p->nextdate) poll(p);
    }

    /*
     * Once per hour, write the clock frequency to a file.
     */
    /*
     * if (c.t % 3600 == 3599)
     *   write c.freq to file
     */
}
