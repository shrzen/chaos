int cheoutput(struct chxcvr *xcvr, register struct packet *pkt, int head);
int cheinit(void);
int chereset(void);
int chestart(struct chxcvr *x);
int cheaddr(char *addr);
void chedeinit(void);
