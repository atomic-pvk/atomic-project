#ifndef NTP_MAIN_UTILITY_H
#define NTP_MAIN_UTILITY_H

#include "NTP_TDMG.h"

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