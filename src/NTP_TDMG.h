/*
NTP_TDMG.h
T Types
D Data Structures
M Macros
G Global Constants
*/
#ifndef NTP_TDMG_H
#define NTP_TDMG_H

#include <math.h>     /* avoids complaints about sqrt() */
#include <stdlib.h>   /* for malloc() and friends */
#include <string.h>   /* for memset() */
#include <sys/time.h> /* for gettimeofday() and friends */

#include "FreeRTOS_IP.h"
#include "FreeRTOS_Sockets.h"
#include "semphr.h"

/*
 * Data types
 *
 * This program assumes the int data type is 32 bits and the long data
 * type is 64 bits.  The native data type used in most calculations is
 * floating double.  The data types used in some packet header fields
 * require conversion to and from this representation.  Some header
 * fields involve partitioning an octet, here represented by individual
 * octets.
 *
 * The 64-bit NTP timestamp format used in timestamp calculations is
 * unsigned seconds and fraction with the decimal point to the left of
 * bit 32.  The only operation permitted with these values is
 * subtraction, yielding a signed 31-bit difference.  The 32-bit NTP
 * short format used in delay and dispersion calculations is seconds and
 * fraction with the decimal point to the left of bit 16.  The only
 * operations permitted with these values are addition and
 * multiplication by a constant.
 *
 * The IPv4 address is 32 bits, while the IPv6 address is 128 bits.  The
 * message digest field is 128 bits as constructed by the MD5 algorithm.
 * The precision and poll interval fields are signed log2 seconds.
 */

typedef uint64_t
    tstamp; /* NTP timestamp format, the first 32 bits represent seconds, the last 32 bits represent fractions */
typedef uint32_t tdist; /* NTP short format */

typedef uint32_t ipaddr; /* IPv4 or IPv6 address */

typedef uint32_t digest; /* md5 digest */
typedef int8_t s_char;   /* precision and poll interval (log2) */

/*
 * Definitions
 */
#define PRECISION -18 /* precision (log2 s)  */
#define IPADDR 0      /* any IP address */
#define MODE 3        /* any NTP mode */
#define KEYID 0       /* any key identifier */

/*
 * Timestamp conversion macroni
 */
#define FRIC 65536.                 /* 2^16 as a double */
#define D2FP(r) ((tdist)((r)*FRIC)) /* NTP short */
#define FP2D(r) ((double)(r) / FRIC)

#define FRAC 4294967296.              /* 2^32 as a double */
#define D2LFP(a) ((tstamp)((a)*FRAC)) /* NTP timestamp */
#define LFP2D(a) ((double)(a) / FRAC)
#define U2LFP(a) (((unsigned long long)((a).tv_sec + JAN_1970) << 32) + (unsigned long long)((a).tv_usec / 1e6 * FRAC))

/*
 * Arithmetic conversions
 */
#define LOG2D(a) ((a) < 0 ? (1. / (1L << -(a))) : (1L << (a))) /* poll, etc. */
#define SQUARE(x) (x * x)
#define SQRT(x) (sqrt(x))
// TODO THIS IS A DUMMY IMPLEMENTATION JUST TO MAKE THE CODE COMPILE
// #define SQRT(x) (x)

/*
 * Global constants.  Some of these might be converted to variables
 * that can be tinkered by configuration or computed on-the-fly.  For
 * instance, the reference implementation computes PRECISION on-the-fly
 * and provides performance tuning for the defines marked with % below.
 */
#define VERSION 4     /* version number */
#define MINDISP .01   /* % minimum dispersion (s) */
#define MAXDISP 16    /* maximum dispersion (s) */
#define MAXDISPAlg .5 /* maximum dispersion (s), a maxdisp of 16 seconds absolutely destroys the algs */
#define MAXDIST 5     /* % distance threshold (s) */
#define NOSYNC 0x3    /* leap unsync */
#define MAXSTRAT 16   /* maximum stratum (infinity metric) */
#define MINPOLL 6     /* % minimum poll interval (64 s)*/
#define MAXPOLL 17    /* % maximum poll interval (36.4 h) */
#define MINCLOCK 3    /* minimum manycast survivors */
#define MAXCLOCK 10   /* maximum manycast candidates */
#define TTLMAX 8      /* max ttl manycast */
#define BEACON 15     /* max interval between beacons */

// this is FreeRTOS_inet_addr("172.21.0.3")
#define DSTADDR 50337196 /* destination (local) address */

#define PHI 15e-6 /* % frequency tolerance (15 ppm) */
#define NSTAGE 8  /* clock register stages */
#define NMAX 5    /* maximum number of peers (number of ntp servers) */
#define NSANE 1   /* % minimum intersection survivors */
#define NMIN 3    /* % minimum cluster survivors */

/*
 * Global return values
 */
#define TRUE 1  /* boolean true */
#define FALSE 0 /* boolean false */

/*
 * Local clock process return codes
 */
#define IGNORE 0 /* ignore */
#define SLEW 1   /* slew adjustment */
#define STEP 2   /* step adjustment */
#define PANIC 3  /* panic - no adjustment */

/*
 * System flags
 */
#define S_FLAGS 0      /* any system flags */
#define S_BCSTENAB 0x1 /* enable broadcast client */

/*
 * Peer flags
 */
#define P_FLAGS 0      /* any peer flags */
#define P_EPHEM 0x01   /* association is ephemeral */
#define P_BURST 0x02   /* burst enable */
#define P_IBURST 0x04  /* intial burst enable */
#define P_NOTRUST 0x08 /* authenticated access */
#define P_NOPEER 0x10  /* authenticated mobilization */
#define P_MANY 0x20    /* manycast client */

/*
 * Authentication codes
 */
#define A_NONE 0   /* no authentication */
#define A_OK 1     /* authentication OK */
#define A_ERROR 2  /* authentication error */
#define A_CRYPTO 3 /* crypto-NAK */

/*
 * Association state codes
 */
#define X_INIT 0   /* initialization */
#define X_STALE 1  /* timeout */
#define X_STEP 2   /* time step */
#define X_ERROR 3  /* authentication error */
#define X_CRYPTO 4 /* crypto-NAK received */
#define X_NKEY 5   /* untrusted key */

/*
 * Protocol mode definitions
 */
#define M_RSVD 0 /* reserved */
#define M_SACT 1 /* symmetric active */
#define M_PASV 2 /* symmetric passive */
#define M_CLNT 3 /* client */
#define M_SERV 4 /* server */
#define M_BCST 5 /* broadcast server */
#define M_BCLN 6 /* broadcast client */

/*
 * Clock state definitions
 */
#define NSET 0 /* clock never set */
#define FSET 1 /* frequency set from file */
#define SPIK 2 /* spike detected */
#define FREQ 3 /* frequency mode */
#define SYNC 4 /* clock synchronized */

#define min(a, b) ((a) < (b) ? (a) : (b))
#define max(a, b) ((a) < (b) ? (b) : (a))

/*
 * The receive and transmit packets may contain an optional message
 * authentication code (MAC) consisting of a key identifier (keyid) and
 * message digest (mac in the receive structure and dgst in the transmit
 * structure).  NTPv4 supports optional extension fields that
 * are inserted after the header and before the MAC, but these are
 * not described here.
 *
 * Receive packet
 *
 * Note the dst timestamp is not part of the packet itself.  It is
 * captured upon arrival and returned in the receive buffer along with
 * the buffer length and data.  Note that some of the char fields are
 * packed in the actual header, but the details are omitted here.
 */
typedef struct ntp_r
{
    // Address info, wait with setting
    uint32_t srcaddr; /* source (remote) address, change from ipaddr to uint32_t */
    uint32_t dstaddr; /* destination (local) address, change from ipaddr to uint32_t */

    //  To implement/initiate
    char version;     /* version number */
    char leap;        /* leap indicator */
    char mode;        /* mode */
    char stratum;     /* stratum */
    char poll;        /* poll interval */
    s_char precision; /* precision */
    tdist rootdelay;  /* root delay */
    tdist rootdisp;   /* root dispersion */
    tdist refid;      /* reference ID */
    tstamp reftime;   /* reference time */
    tstamp org;       /* origin timestamp */
    tstamp rec;       /* receive timestamp */
    tstamp xmt;       /* transmit timestamp */

    // Crypto stuff, wait with TODO?
    int keyid;  /* key ID */
    digest mac; /* message digest */
    tstamp dst; /* destination timestamp */
} ntp_r;

/*
 * Transmit packet
 */
typedef struct ntp_x
{
    uint32_t srcaddr; /* source (remote) address, change from ipaddr to uint32_t */
    uint32_t dstaddr; /* destination (local) address, change from ipaddr to uint32_t */
    char version;     /* version number */
    char leap;        /* leap indicator */
    char mode;        /* mode */
    char stratum;     /* stratum */
    char poll;        /* poll interval */
    s_char precision; /* precision */
    tdist rootdelay;  /* root delay */
    tdist rootdisp;   /* root dispersion */
    tdist refid;      /* reference ID */
    tstamp reftime;   /* reference time */
    tstamp org;       /* origin timestamp */
    tstamp rec;       /* receive timestamp */
    tstamp xmt;       /* transmit timestamp */
    int keyid;        /* key ID */
    digest dgst;      /* message digest */
} ntp_x;

/*
 * Filter stage structure.  Note the t member in this and other
 * structures refers to process time, not real time.  Process time
 * increments by one second for every elapsed second of real time.
 */
typedef struct ntp_f
{
    tstamp t;      /* update time */
    double offset; /* clock ofset */
    double delay;  /* roundtrip delay */
    double disp;   /* dispersion */
} ntp_f;

/*
 * Association structure.  This is shared between the peer process
 * and poll process.
 */
typedef struct ntp_p
{
    /*
     * Variables set by configuration
     */
    uint32_t srcaddr; /* source (remote) address, change from ipaddr to uint32_t */
    uint32_t dstaddr; /* destination (local) address, change from ipaddr to uint32_t */
    char version;     /* version number */
    char hmode;       /* host mode */
    int keyid;        /* key identifier */
    int flags;        /* option flags */

    /*
     * Variables set by received packet
     */
    char leap;          /* leap indicator */
    char pmode;         /* peer mode */
    char stratum;       /* stratum */
    char ppoll;         /* peer poll interval */
    double rootdelay;   /* root delay */
    double rootdisp;    /* root dispersion */
    tdist refid;        /* reference ID */
    tstamp reftime;     /* reference time */
#define begin_clear org /* beginning of clear area */
    tstamp org;         /* originate timestamp */
    tstamp rec;         /* receive timestamp */
    tstamp xmt;         /* transmit timestamp */

    /*
     * Computed data
     */
    tstamp t;               /* update time */
    struct ntp_f f[NSTAGE]; /* clock filter */
    double offset;          /* peer offset */
    double delay;           /* peer delay */
    double disp;            /* peer dispersion */
    double jitter;          /* RMS jitter */

    /*
     * Poll process variables
     */
    char hpoll;           /* host poll interval */
    int burst;            /* burst counter */
    int reach;            /* reach register */
    int ttl;              /* ttl (manycast) */
#define end_clear unreach /* end of clear area */
    int unreach;          /* unreach counter */
    int outdate;          /* last poll time */
    int nextdate;         /* next poll time */
} ntp_p;

/*
 * Chime list structure based on Marzullo's algorithm, used in the intersection algorithm of NTP.
 *
 * This structure represents an entry in the chime list.
 */
typedef struct ntp_m
{
    struct ntp_p *p;
    int type;
    double edge;
} ntp_m;

/*
 * Survivor list structure used in the clustering algorithm of NTP.
 *
 * This structure represents an entry in the survivor list.
 */
typedef struct ntp_v
{
    struct ntp_p *p;
    double metric;
} ntp_v;

/*
 * NTP peer structure based on RFC 5905.
 *
 * This structure represents an NTP peer in the system.
 */
typedef struct ntp_s
{
    tstamp t;                 /* update time */
    char leap;                /* leap indicator */
    char stratum;             /* stratum */
    char poll;                /* poll interval */
    s_char precision;         /* precision */
    double rootdelay;         /* root delay */
    double rootdisp;          /* root dispersion */
    tdist refid;              /* reference ID */
    tstamp reftime;           /* reference time */
    struct ntp_m m[NMAX * 5]; /* chime list */
    struct ntp_v v[NMAX * 5]; /* survivor list */
    struct ntp_p *p;          /* association ID */
    double offset;            /* combined offset */
    double jitter;            /* combined jitter */
    int flags;                /* option flags */
    int n;                    /* number of survivors */
} ntp_s;

/*
 * Local clock structure based on RFC 5905.
 *
 * This structure represents the local clock in an NTP system.
 */
typedef struct ntp_c
{
    tstamp localTime;             /* local based on ticks time */
    tstamp t;                     /* update time */
    int state;                    /* current state */
    double offset;                /* current offset */
    double last;                  /* previous offset */
    int count;                    /* jiggle counter */
    double freq;                  /* frequency */
    double jitter;                /* RMS jitter */
    double wander;                /* RMS wander */
    TickType_t lastTimeStampTick; /* freertos tick*/
} ntp_c;

// Association Data Structures used in the assoc table to find associations between peers and addresses
typedef struct Assoc_table
{
    ntp_p **peers;
    int size;
} Assoc_table;

void assoc_table_init(uint32_t *);
int assoc_table_add(uint32_t, char, tstamp);
void assoc_table_update(ntp_p *);

double sqrt(double number);

int64_t subtract_uint64_t(uint64_t x, uint64_t y);
int64_t add_int64_t(int64_t x, int64_t y);

void settime(tstamp newTime);
void gettime(int override);

void printTimestamp(tstamp timestamp, const char *comment);

void uint64_to_str(uint64_t value, char *str);
void double_to_str(double value, char *str, int precision);
void FreeRTOS_printf_wrapper(const char *format, uint64_t value);
void FreeRTOS_printf_wrapper_double(const char *format, double value);
void print_uint64_as_32_parts(uint64_t);

#endif