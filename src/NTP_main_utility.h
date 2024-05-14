#ifndef NTP_MAIN_UTILITY_H
#define NTP_MAIN_UTILITY_H

#include "NTP_TDMG.h"

typedef struct ntp_packet
{
    uint8_t li_vn_mode;       // Eight bits. li, vn, and mode.
                              // li.   Two bits.   Leap indicator.
                              // vn.   Three bits. Version number of the protocol.
                              // mode. Three bits. Client will pick mode 3 for client.
    uint8_t stratum;          // Eight bits. Stratum level of the local clock.
    uint8_t poll;             // Eight bits. Maximum interval between successive messages.
    uint8_t precision;        // Eight bits. Precision of the local clock.
    uint32_t rootDelay;       // 32 bits. Total round trip delay time.
    uint32_t rootDispersion;  // 32 bits. Max error aloud from primary clock source.
    uint32_t refId;           // 32 bits. Reference clock identifier.
    uint32_t refTm_s;         // 32 bits. Reference time-stamp seconds.
    uint32_t refTm_f;         // 32 bits. Reference time-stamp fraction of a second.
    uint32_t origTm_s;        // 32 bits. Originate time-stamp seconds.
    uint32_t origTm_f;        // 32 bits. Originate time-stamp fraction of a second.
    uint32_t rxTm_s;          // 32 bits. Received time-stamp seconds.
    uint32_t rxTm_f;          // 32 bits. Received time-stamp fraction of a second.
    uint32_t txTm_s;          // 32 bits. Transmit time-stamp seconds.
    uint32_t txTm_f;          // 32 bits. Transmit time-stamp fraction of a second.
} ntp_packet;

// mobilize - allocates and initializes a new peer structure
ntp_p *mobilize(uint32_t srcaddr,  // IP source address
                uint32_t dstaddr,  // IP destination address
                int version,       // version
                int mode,          // host mode
                int keyid,         // key identifier
                int flags          // peer flags
);

// find_assoc - find a matching association
ntp_p *find_assoc(ntp_r *r  // receive packet pointer
);

// md5 - compute message digest
digest md5(int keyid  // key identifier
);

// Kernel Input/Output Interface

void prep_xmit(ntp_x *);  // prepare packet

// recv_packet - receive packet from network
void recv_packet(ntp_r *);

// xmit_packet - transmit packet to network
void xmit_packet(ntp_x *x  // transmit packet pointer
);

// Kernel System Clock Interface

// get_time - read system time and convert to NTP format
tstamp get_time(void);

// step_time - step system time to given offset value
void step_time(double offset  // clock offset
);

// adjust_time - slew system clock to given offset value
void adjust_time(double offset  // clock offset
);

#endif /* NTP_MAIN_UTILITY_H */