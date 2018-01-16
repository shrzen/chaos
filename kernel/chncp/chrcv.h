#ifndef CHAOS_CHNCP_CHRCV_H
#define CHAOS_CHNCP_CHRCV_H

void rcvpkt(struct chxcvr *xp);
void reflect(struct packet *pkt);
void rcvdata(struct connection *conn, struct packet *pkt);
void makests(struct connection *conn, struct packet *pkt);
void receipt(struct connection *conn,unsigned short acknum, unsigned short recnum);
void sendlos(struct packet *pkt,char *str,	int len);
void rcvbrd(struct packet *pkt);
void rcvrfc(struct packet *pkt);
void lsnmatch(struct packet *rfcpkt,struct connection *conn);
void rmlisten(struct connection *conn);
int concmp(struct packet *rfcpkt,char *lsnstr,int lsnlen);
void rcvrut(struct packet *pkt);
void statusrfc(struct packet *pkt);
void dumprtrfc(struct packet *pkt);
void timerfc(struct packet *pkt);
void uptimerfc(struct packet *pkt);
void prpkt(struct packet *pkt,char *str);
void showpkt(char *str, struct packet *pkt);

#endif
