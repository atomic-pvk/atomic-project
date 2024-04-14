#ifndef NTP_SYSTEM_PROCESS_H
#define NTP_SYSTEM_PROCESS_H

#include "NTP_TDMG.h"

int main();                        /* main program */
void clock_select();               /* find the best clocks */
void clock_update(struct ntp_p *); /* update the system clock */
void clock_combine();              /* combine the offsets */

#endif /* NTP_SYSTEM_PROCESS_H */