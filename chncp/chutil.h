struct connection *allconn(void);
void clsconn(struct connection *conn,int state,struct packet *pkt);
void rlsconn(struct connection *conn);
void freelist(struct packet *pkt);
void clear(char *ptr,int n);
void movepkt(struct packet *opkt, struct packet *npkt);
void chmove(char *from, char *to,int n);
void setpkt(struct connection *conn,struct packet *pkt);
void chattach(struct chxcvr *xp);
