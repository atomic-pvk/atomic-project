#ifndef NTP_PEER_H
#define NTP_PEER_H

#include "NTP_TDMG.h"

/*
 * A crypto-NAK packet includes the NTP header followed by a MAC
 * consisting only of the key identifier with value zero.  It tells
 * the receiver that a prior request could not be properly
 * authenticated, but the NTP header fields are correct.
 *
 * A kiss-o'-death packet is an NTP header with leap 0x3 (NOSYNC) and
 * stratum 16 (MAXSTRAT).  It tells the receiver that something
 * drastic has happened, as revealed by the kiss code in the refid
 * field.  The NTP header fields may or may not be correct.
 */
/*
 * Peer process parameters and constants
 */
#define SGATE 3     /* spike gate (clock filter */
#define BDELAY .004 /* broadcast delay (s) */

/*
 * Dispatch codes
 */
#define ERR -1  /* error */
#define DSCRD 0 /* discard packet */
#define PROC 1  /* process packet */
#define BCST 2  /* broadcast packet */
#define FXMIT 3 /* client packet */
#define MANY 4  /* manycast packet */
#define NEWPS 5 /* new symmetric passive client */
#define NEWBC 6 /* new broadcast client */

/*
 * Miscellaneous macroni
 *
 * This macro defines the authentication state.  If x is 0,
 * authentication is optional; otherwise, it is required.
 */
#define AUTH(x, y) ((x) ? (y) == A_OK : (y) == A_OK || (y) == A_NONE)

/*
 * These are used by the clear() routine
 */
#define BEGIN_CLEAR(p) ((char *)&((p)->begin_clear))
#define END_CLEAR(p) ((char *)&((p)->end_clear))
#define LEN_CLEAR (END_CLEAR((struct ntp_p *)0) - \
                   BEGIN_CLEAR((struct ntp_p *)0))

void receive(struct ntp_r *);                              /* receive packet */
void packet(struct ntp_p *, struct ntp_r *);               /* process packet */
void clock_filter(struct ntp_p *, double, double, double); /* filter */
int fit(struct ntp_p *);                                   /* determine fitness of server */
void clear(struct ntp_p *, int);                           /* clear association */
int access(struct ntp_r *r);

#endif /* NTP_PEER_H */