void sendsts(struct connection *conn);
void sendsns(struct connection *conn);
void sendctl(struct packet *pkt);
void senddata(struct packet *pkt);
void sendrut(struct packet *pkt,struct chxcvr *xcvr,unsigned short cost,int copy);
void sendtome(struct packet *pkt);
void xmitdone(struct packet *pkt);
struct packet *xmitnext(struct chxcvr *xcvr);
