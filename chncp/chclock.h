#ifndef CHAOS_CHNCP_CHCLOCK_H
#define CHAOS_CHNCP_CHCLOCK_H

void ch_clock(void);
void chretran(struct connection *conn,int age);
void chroutage(void);
int chbridge(void);

#endif
