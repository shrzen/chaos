///---!!! Much of this is in ../chncp, ../chunix, we should use that
///---!!!   code instead of duplicating it here.  It is also shared
///---!!!   with the kernel driver.

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>

#include <sys/types.h>
#include <sys/uio.h>

#include "../chunix/chsys-socket.h"
#include "../h/chaos.h"
#include "../chunix/chconf.h"
#include "../chncp/chncp.h"
#include "log.h"

struct chroute  Chroutetab[CHNSUBNET];
struct connection *Chconntab[CHNCONNS];
short Chmyaddr;
char Chmyname[32];

int Chnobridge;
int Chrfcrcv;
chtime Chclock;
int Cherror;
int Chaos_error;
int Chhz;

struct packet *Chlsnlist;	/* listening connections */

struct packet *Chrfclist;
struct packet *Chrfctail;

#define INPUT(x) server_input(x)
#define OUTPUT(x)
#define NEWSTATE(x)
#define RFCINPUT

void senddata(struct packet *pkt);
void rcvpkt(struct chxcvr *xp);

/*
 * Zero out n bytes
 */
void
clear(char *ptr, int n)
{
	if (n)
		do {
			*ptr++ = 0;
		} while (--n);
}

/*
 * Move contents of opkt to npkt
 */
void
movepkt(struct packet *opkt, struct packet *npkt)
{
	short *nptr, *optr, n;

	n = (CHHEADSIZE + PH_LEN(opkt->pk_phead) + 
	     sizeof(short) - 1) / sizeof(short);
	nptr = (short *)npkt;
	optr = (short *)opkt;
	do {
		*nptr++ = *optr++;
	} while (--n);
}

/*
 * Move n bytes
 */
void
chmove(char *from, char *to, int n)
{
	if (n)
		do *to++ = *from++; while(--n);
}


struct packet *
ch_alloc_pkt(int data_size)
{
	struct packet *pkt;
	int alloc_size = sizeof(struct packet) + data_size;

	tracef(TRACE_LOW, "ch_alloc_pkt(size=%d)", data_size);

	pkt = (struct packet *)malloc(alloc_size);
	if (pkt == 0)
		return NULL;

	if (1) memset((char *)pkt, 0, alloc_size);

	return pkt;
}

void
ch_free_pkt(struct packet *pkt)
{
	free((char *)pkt);
}

/*
 * Free a list of packets
 */
void
freelist(struct packet *pkt)
{
	struct packet *opkt;

	while ((opkt = pkt) != NOPKT) {
		pkt = pkt->pk_next;
		ch_free_pkt(opkt);
	}
}

/*
 * Fill a packet with a string, returning packet because it may reallocate
 * If the pkt argument is NOPKT then allocate a packet here.
 * The string is null terminated and may be shorter than "len".
 */
struct packet *
pktstr(struct packet *pkt, char *str, int len)
{
	struct packet *npkt;
	register char *odata;

	if (pkt == NOPKT /*|| ch_size((char *)pkt) < CHHEADSIZE + len*/ ) {
		if ((npkt = ch_alloc_pkt(len)) == NOPKT)
			return(NOPKT);
		if (pkt != NOPKT) {
		  SET_PH_LEN(pkt->pk_phead,0);
			movepkt(pkt, npkt);
			ch_free_pkt(pkt);
		}
		pkt = npkt;
	}
	odata = pkt->pk_cdata;
	SET_PH_LEN(pkt->pk_phead,len);
	if (len) do *odata++ = *str; while (*str++ && --len);
	return(pkt);
}

static int uniq;

/*
 * Allocate a connection and return it, also allocating a slot in Chconntab
 */
struct connection *
allconn(void)
{
	struct connection *conn;
	struct connection **cptr;
//	static int uniq;

	if ((conn = (struct connection *)malloc(sizeof(struct connection))) == NOCONN) {
		debugf(DBG_WARN, "allconn: alloc failed (connection)");
		Chaos_error = CHNOPKT;
		return(NOCONN);
	}

	for(cptr = &Chconntab[0]; cptr != &Chconntab[CHNCONNS]; cptr++) {
		if(*cptr != NOCONN) continue;
		*cptr = conn;
		clear((char *)conn, sizeof(struct connection));
		SET_CH_INDEX(conn->cn_Lidx,cptr - &Chconntab[0]);
		if (++uniq == 0)
			uniq = 1;
		conn->cn_luniq = uniq;
		debugf(DBG_LOW, "allconn: alloc #%x", 
		       CH_INDEX_SHORT(conn->cn_Lidx));
		return(conn);
	}

	free((char *)conn);
	Chaos_error = CHNOCONN;
	debugf(DBG_WARN, "allconn: alloc failed (table)");

	return(NOCONN);
}

/*
 * Make a connection closed with given state, at interrupt time.
 * Queue the given packet on the input queue for the user.
 */
void
clsconn(struct connection *conn, int state, struct packet *pkt)
{
	freelist(conn->cn_thead);
	conn->cn_thead = conn->cn_ttail = NOPKT;
	conn->cn_state = state;
	debugf(DBG_LOW, "clsconn: Conn #%x: CLOSED: state: %d",
	       CH_INDEX_SHORT(conn->cn_Lidx), state);
	if (pkt != NOPKT) {
		pkt->pk_next = NOPKT;
		if (conn->cn_rhead != NOPKT)
			conn->cn_rtail->pk_next = pkt;
		else
			conn->cn_rhead = pkt;
		conn->cn_rtail = pkt;
	}
	NEWSTATE(conn);
}
	
/*
 * Release a connection - freeing all associated storage.
 * This removes all trace of the connection.
 * Always called from top level at low priority.
 */
void
rlsconn(struct connection *conn)
{
	Chconntab[conn->cn_ltidx] = NOCONN;
	freelist(conn->cn_routorder);
	freelist(conn->cn_rhead);
	freelist(conn->cn_thead);

	debugf(DBG_LOW, "rlsconn: release #%x", CH_INDEX_SHORT(conn->cn_Lidx));
	free((char *)conn);
}

char *opcodetable[256] = {
    "UNKNOWN",			// 0
    "RFC",			// 1
    "OPN",			// 2
    "CLS",			// 3
    "FWD",			// 4
    "ANS",			// 5
    "SNS",			// 6
    "STS",			// 7
    "RUT",			// 8
    "LOS",			// 9
    "LSN",			// 10
    "MNT",			// 11
    "EOF",			// 12
    "UNC",                      // 13
    "BRD",                      // 14
    "UNKNOWN",                  // 15
};

void
prpkt(struct packet *pkt, char *str)
{
	debugf(DBG_LOW, "op=%s(%s) num=%d len=%d fc=%d; dhost=%o didx=%x; shost=%o sidx=%x\npkn=%d ackn=%d",
		str, pkt->pk_op == 128 ? "DAT" : pkt->pk_op == 129 ? "SYN" : pkt->pk_op == 192 ? "DWD" : opcodetable[pkt->pk_op],
		LE_TO_SHORT(pkt->LE_pk_pkn), PH_LEN(pkt->pk_phead), 
	        PH_FC(pkt->pk_phead), pkt->pk_dhost,
		CH_INDEX_SHORT(pkt->pk_phead.ph_didx), pkt->pk_shost, 
	        CH_INDEX_SHORT(pkt->pk_phead.ph_sidx),
	       (unsigned)LE_TO_SHORT(pkt->LE_pk_pkn), 
	       (unsigned)LE_TO_SHORT(pkt->LE_pk_ackn));
}

/*
 * Make a STS packet for a connection, reflecting its current state
 */
void
makests(struct connection *conn, struct packet *pkt)
{
	pkt->pk_op = STSOP;
	SET_PH_LEN(pkt->pk_phead, sizeof(struct sts_data));
	setack(conn, pkt);
	pkt->LE_pk_receipt = LE_TO_SHORT(conn->cn_rlast);
	pkt->LE_pk_rwsize = LE_TO_SHORT(conn->cn_rwsize);
}


/*
 * Process receipts and acknowledgements using recnum as the receipt.
 */
void
receipt(struct connection *conn, unsigned short acknum, unsigned short recnum)
{
	struct packet *pkt, *pktl;

	/*
	 * Process a receipt, freeing packets that we now know have been
	 * received.
	 */
	if (cmp_gt(recnum, conn->cn_trecvd)) {
		for (pktl = conn->cn_thead;
		     pktl != NOPKT && cmp_le(LE_TO_SHORT(pktl->LE_pk_pkn), 
					     recnum);
		     pktl = pkt)
		{
			pkt = pktl->pk_next;
			ch_free_pkt(pktl);
		}
		if ((conn->cn_thead = pktl) == NOPKT)
			conn->cn_ttail = NOPKT;
		conn->cn_trecvd = recnum;
	}

	/*
	 * If the acknowledgement is new, update our idea of the
	 * latest acknowledged packet, and wakeup output that might be blocked
	 * on a full transmit window.
	 */
	if (cmp_gt(acknum, conn->cn_tacked))
		if (cmp_gt(acknum, conn->cn_tlast))
			debugf(DBG_WARN,
			       "receipt: Invalid acknowledgment(%d,%d)",
			       acknum, conn->cn_tlast);
		else {
			conn->cn_tacked = acknum;
			OUTPUT(conn);
		}
}

/*
 * Indicate that actual transmission of the current packet has been completed.
 * Called by the device dependent interrupt routine when transmission
 * of a packet has finished.
 */
void
xmitdone(struct packet *pkt)
{
	struct connection *conn;
	struct packet *npkt;

	if (PH_FC(pkt->pk_phead) == 0 && CONTPKT(pkt) &&
	    pkt->pk_stindex < CHNCONNS &&
	    (conn = Chconntab[pkt->pk_stindex]) != NOCONN &&
	    CH_INDEX_SHORT(pkt->pk_sidx) == 
	    CH_INDEX_SHORT(conn->cn_Lidx) &&
	    (conn->cn_state == CSOPEN || conn->cn_state == CSRFCSENT) &&
	    cmp_gt(LE_TO_SHORT(pkt->LE_pk_pkn), conn->cn_trecvd))
	{
		pkt->pk_time = Chclock;

		if ((npkt = conn->cn_thead) == NOPKT || 
		    cmp_lt(LE_TO_SHORT(pkt->LE_pk_pkn), 
			   LE_TO_SHORT(npkt->LE_pk_pkn)))
		{
			pkt->pk_next = npkt;
			conn->cn_thead = pkt;
		} else {
			for( ; npkt->pk_next != NOPKT; npkt = npkt->pk_next)
				if(cmp_lt(LE_TO_SHORT(pkt->LE_pk_pkn), 
					  LE_TO_SHORT(npkt->pk_next->LE_pk_pkn)))
					break;
			pkt->pk_next = npkt->pk_next;
			npkt->pk_next = pkt;
		}
		if(pkt->pk_next == NOPKT)
			conn->cn_ttail = pkt;
	} else
		ch_free_pkt(pkt);
}

/*
 * Return the next packet on which to begin transmission (if none,  NOPKT).
 */
struct packet *
xmitnext(struct chxcvr *xcvr)
{
	struct packet *pkt;

	if ((pkt = xcvr->xc_tpkt = xcvr->xc_list) != NOPKT) {
		if ((xcvr->xc_list = pkt->pk_next) == NOPKT)
			xcvr->xc_tail = NOPKT;
		xcvr->xc_ttime = Chclock;
	}

	return pkt;
}

void
chretran(struct connection *conn, int age)
{
	struct packet *pkt, **opkt;
	struct packet *lastpkt;
	struct packet *firstpkt = NOPKT;

	for (opkt = &conn->cn_thead; pkt = *opkt;)
		if (cmp_gt(Chclock, pkt->pk_time + age)) {
			if (firstpkt == NOPKT) 
				firstpkt = pkt;
			else
				lastpkt->pk_next = pkt;

			lastpkt = pkt;
			*opkt = pkt->pk_next;
			pkt->pk_next = NOPKT;
			setack(conn, pkt);
		} else
			opkt = &pkt->pk_next;

	if (firstpkt != NOPKT) {
		debugf(DBG_LOW, "chretran: Conn #%x: Rexmit (op:%d, pkn:%d)",
		       CH_INDEX_SHORT(conn->cn_Lidx), firstpkt->pk_op,
		       LE_TO_SHORT(firstpkt->LE_pk_pkn));
		senddata(firstpkt);
	}
}

/*
 * Send the (list of) packet(s) to myself - NOTE THIS CAN BE RECURSIVE!
 */
void
sendtome(register struct packet *pkt)
{
	struct packet *rpkt, *npkt;
	static struct chxcvr fakexcvr;

	tracef(TRACE_LOW, "sendtome()");

	/*
	 * Static structure is used to economize on stack space.
	 * We are careful to use it very locally so that recursion still
	 * works. Cleaner solutions don't recurse very well.
	 * Basically for each packet in the list, prepare it for
	 * transmission by executing xmitnext,
	 * complete the transmission by calling xmitdone,
	 * then receive the packet on the other side, possibly
	 * causing this routine to recurse (after we're done with the
	 * static structure until next time around the loop)
	 * When this routine is called with a list of data packets, things
	 * can get pretty weird.
	 */
	while (pkt != NOPKT) {
		/*
		 * Make the transmit list consist of one packet to send.
		 * Save the rest of the list in npkt.
		 */
		npkt = pkt->pk_next;
		pkt->pk_next = NOPKT;
		fakexcvr.xc_tpkt = NOPKT;
		fakexcvr.xc_list = fakexcvr.xc_tail = pkt;
		/*
		 * The xmitnext just dequeues pkt and sets ackn.
		 * It will free the packet if its not worth sending.
		 */
		(void)xmitnext(&fakexcvr);
		if (fakexcvr.xc_tpkt) {
			/*
			 * So it really should be sent.
			 * First make a copy for the receiving side in rpkt.
			 */
			if ((rpkt = ch_alloc_pkt(PH_LEN(pkt->pk_phead))) 
			    != NOPKT)
				movepkt(pkt, rpkt);
			/*
			 * This xmitdone just completes transmission.
			 * Now pkt is out of our hands.
			 */
			xmitdone(pkt);
			/*
			 * So transmission is complete. Now receive it.
			 */
			if (rpkt != NOPKT) {
				fakexcvr.xc_rpkt = rpkt;
				rcvpkt(&fakexcvr);
			}
		}
		pkt = npkt;
	}
}

/*
 * Sendctl - send a control packet that will not be acknowledged and
 *	will not be retransmitted (this actual packet).  Put the
 *	given packet at the head of the transmit queue so it is transmitted
 *	"quickly", i.e. before data packets queued at the tail of the queue.
 *	If anything is wrong (no path, bad subnet) we just throw it
 *	away. Remember, pk_next is assumed to be == NOPKT.
 */
void
sendctl(struct packet *pkt)
{
	struct chroute *r;

	if (1) {
		debugf(DBG_LOW, "chaos: sendctl: Sending: %d ", pkt->pk_op);
		prpkt(pkt, "ctl");
		debugf(DBG_LOW, "\n");
	}
	if (CH_ADDR_SHORT(pkt->pk_daddr) == Chmyaddr) {
		debugf(DBG_LOW, "to me");
		sendtome(pkt);
	} else if (pkt->pk_dsubnet >= CHNSUBNET ||
	    (r = &Chroutetab[pkt->pk_dsubnet])->rt_type == CHNOPATH ||
	     r->rt_cost >= CHHCOST) {
		debugf(DBG_LOW, "chaos: Dropping, no route");
		ch_free_pkt(pkt);
	} else {
		if (r->rt_type == CHFIXED || r->rt_type == CHBRIDGE) {
			pkt->pk_xdest = r->rt_addr;
			r = &Chroutetab[r->rt_subnet];
		} else
			pkt->pk_xdest = pkt->pk_daddr;
		(*r->rt_xcvr->xc_xmit)(r->rt_xcvr, pkt, 1);
	}
}

/*
 * Senddata - send a list of controlled packets, all assumed to be to the
 *	same destination.  Queue them on the end of the appropriate transmit
 *	queue.
 *	Similar to sendctl, but stuffs time, handles a list, and puts pkts
 *	at the end of the queue instead of at the beginning, and fakes
 *	transmission if it can't really send the packets (as opposed to
 *	throwing the packets away)
 */
void
senddata(struct packet *pkt)
{
	struct chroute *r;

	if (1) {
		debugf(DBG_LOW, "Sending: %d ", pkt->pk_op);
		prpkt(pkt, "data");
		debugf(DBG_LOW, "\n");
	}

	if (CH_ADDR_SHORT(pkt->pk_daddr) == Chmyaddr) {
		debugf(DBG_LOW, "to me");
		sendtome(pkt);
        }
	else if (pkt->pk_dsubnet >= CHNSUBNET ||
	    (r = &Chroutetab[pkt->pk_dsubnet])->rt_type == CHNOPATH ||
	     r->rt_cost >= CHHCOST) {
		struct packet *npkt;
		debugf(DBG_LOW, "no path to 0%x", CH_ADDR_SHORT(pkt->pk_daddr));
		do {
			npkt = pkt->pk_next;
			pkt->pk_next = NOPKT;
			xmitdone(pkt);
		} while ((pkt = npkt) != NOPKT);
	} else {
		struct chxcvr *xcvr;
		unsigned short dest;

		if (r->rt_type == CHFIXED || r->rt_type == CHBRIDGE) {
			dest = CH_ADDR_SHORT(r->rt_addr);
			r = &Chroutetab[r->rt_subnet];
		} else
			dest = CH_ADDR_SHORT(pkt->pk_daddr);
		
		xcvr = r->rt_xcvr;
		for (;;) {
			struct packet *next;

			pkt->pk_time = Chclock;
			SET_CH_ADDR(pkt->pk_xdest,dest);
			next = pkt->pk_next;
			(*xcvr->xc_xmit)(xcvr, pkt, 0);
			if ((pkt = next) == NOPKT)
				break;
		}
	}
}

/*
 * Send a control packet back to its source
 */
void
reflect(struct packet *pkt)
{
	chindex temp_idx;
        chaddr temp_addr;

	temp_idx = pkt->pk_sidx;
	pkt->pk_sidx = pkt->pk_didx;
	pkt->pk_didx = temp_idx;
	temp_addr = pkt->pk_saddr;
	pkt->pk_saddr = pkt->pk_daddr;
	pkt->pk_daddr = temp_addr;

#if 1
	struct pkt_header *ph = (struct pkt_header *)&pkt->pk_phead;

	debugf(DBG_LOW,
	       "reflect: to (%o %o:%d,%d) from (%o %o:%d,%d) pkn %o ackn %o\n",
	       ph->ph_daddr.subnet, ph->ph_daddr.host,
	       ph->ph_didx.tidx, ph->ph_didx.uniq,
	       ph->ph_saddr.subnet, ph->ph_saddr.host,
	       ph->ph_sidx.tidx, ph->ph_sidx.uniq,
	       LE_TO_SHORT(ph->LE_ph_pkn), 
	       LE_TO_SHORT(ph->LE_ph_ackn));
#endif

	sendctl(pkt);
}

#define send_los(x,y) sendlos(x,y,sizeof(y) - 1)

/*
 * Send a LOS in response to the given packet.
 * Don't bother if the packet is itself a LOS or a CLS since the other
 * end doesn't care anyway and would only return it again.
 * Append the host name to the error message.
 */
void
sendlos(struct packet *pkt, char *str, int len)
{
	debugf(DBG_INFO, "sendlos() %s", str);

	if (pkt->pk_op == LOSOP || pkt->pk_op == CLSOP)
		ch_free_pkt(pkt);
	else {
		char *cp;
		int length;
		
		for (cp = Chmyname, length = 0; *cp && length < CHSTATNAME;) {
			length++;
			cp++;
		}
		if (pkt = pktstr(pkt, str, len + length + sizeof("(from )") - 1)) {
			chmove("(from ", &pkt->pk_cdata[len], 6);
			chmove(Chmyname, &pkt->pk_cdata[len + 6], length);
			chmove(")", &pkt->pk_cdata[len + 6 + length], 1);
			pkt->pk_op = LOSOP;
			reflect(pkt);
		}
	}
}

/*
 * An RFC has matched a listener, either by an RFC coming and finding a match
 * on the listener list, or by a listen being done and matching an RFC on the
 * unmatched RFC list. So we change the state of the connection to CSRFCRCVD
 */
void
lsnmatch(struct packet *rfcpkt, struct connection *conn)
{
	debugf(DBG_INFO, "lsnmatch: Conn #%x: LISTEN matched", 
	       CH_INDEX_SHORT(conn->cn_Lidx));

	/*
	 * Initialize the conection
	 */
	conn->cn_active = Chclock;
	conn->cn_state = CSRFCRCVD;
	if (conn->cn_rwsize == 0)
		conn->cn_rwsize = CHDRWSIZE;
	conn->cn_faddr = rfcpkt->pk_saddr;
	conn->cn_Fidx = rfcpkt->pk_sidx;
	if (CH_ADDR_SHORT(rfcpkt->pk_daddr) == 0) {
	  conn->cn_laddr = rfcpkt->pk_daddr;
	} else {
	  SET_CH_ADDR(conn->cn_laddr,Chmyaddr);
	}

	/*
	 * Queue up the RFC for the user to read if he wants it.
	 */
	rfcpkt->pk_next = NOPKT;
	conn->cn_rhead = conn->cn_rtail = rfcpkt;
	/*
	 * Indicate to the other end that we have received and "read"
	 * the RFC so that the open will truly acknowledge it.
	 */
	conn->cn_rlast = conn->cn_rread = LE_TO_SHORT(rfcpkt->LE_pk_pkn);
}

/*
 * Remove a listener from the listener list, due to the listener bailing out.
 * Called from top level at high priority
 */
void
rmlisten(struct connection *conn)
{
	struct packet *opkt, *pkt;

	opkt = NOPKT;
	for (pkt = Chlsnlist; pkt != NOPKT; opkt = pkt, pkt = pkt->pk_next)
		if (pkt->pk_stindex == conn->cn_ltidx) {
			if(opkt == NOPKT)
				Chlsnlist = pkt->pk_next;
			else
				opkt->pk_next = pkt->pk_next;
			ch_free_pkt(pkt);
			break;
		}
}

/*
 * Compare the RFC contact name with the listener name.
 */
int
concmp(struct packet *rfcpkt, char *lsnstr, int lsnlen)
{
	char *rfcstr = rfcpkt->pk_cdata;
	int rfclen;
	char crfcstr[CHMAXDATA];

	memcpy(crfcstr, rfcpkt->pk_cdata, PH_LEN(rfcpkt->pk_phead));
	crfcstr[PH_LEN(rfcpkt->pk_phead)] = '\0';
	debugf(DBG_LOW, "Rcvrfc: Comparing %s and %s", crfcstr, lsnstr);
	debugf(DBG_LOW, "rfcpkt->pk_len = %d lsnlen = %d", 
	       PH_LEN(rfcpkt->pk_phead),
	       lsnlen);

	for (rfclen = PH_LEN(rfcpkt->pk_phead); rfclen; rfclen--, lsnlen--) {
	  if (lsnlen <= 0) {
	    debugf(DBG_LOW, "reached lsnlen = %d *rfcstr is '%c' %x", lsnlen,
		   *rfcstr, *rfcstr);
	    debugf(DBG_LOW, "returning 1");
	    return 1;
	    /* return ((*rfcstr == ' ') ? 1 : 0); */
	  }
	  else if (*rfcstr++ != *lsnstr++) {
	    debugf(DBG_LOW, "failed to match character %x and %x",
		   *(rfcstr-1), *(lsnstr-1));
	    return(0);
	  }
	}

	return (lsnlen == 0);
}

/*
 * Process a routing packet.
 */
void
rcvrut(struct packet *pkt)
{
	int i;
	struct rut_data *rd;
	struct chroute *r;

	if (1) {
		prpkt(pkt,"RUT");
		debugf(DBG_LOW, "\n");
	}

	rd = pkt->pk_rutdata;
	if (pkt->pk_ssubnet >= CHNSUBNET)
		debugf(DBG_LOW, "CHAOS: bad subnet %d in RUT pkt",
		       pkt->pk_ssubnet);
	else if (Chroutetab[pkt->pk_ssubnet].rt_type != CHDIRECT)
		debugf(DBG_LOW, "CHAOS: RUT pkt from unconnected subnet %d",
			pkt->pk_ssubnet);
	else for (i = PH_LEN(pkt->pk_phead); i; i -= sizeof(struct rut_data), rd++) {
		if (LE_TO_SHORT(rd->LE_pk_subnet) >= CHNSUBNET) {
			debugf(DBG_LOW, "CHAOS: bad subnet %d in RUT pkt",
			       LE_TO_SHORT(rd->LE_pk_subnet));
			continue;
		}
		r = &Chroutetab[LE_TO_SHORT(rd->LE_pk_subnet)];
		switch (r->rt_type) {
		case CHNOPATH:
		case CHBRIDGE:
		case CHDIRECT:
			if (LE_TO_SHORT(rd->LE_pk_cost) < r->rt_cost) {
				r->rt_cost = LE_TO_SHORT(rd->LE_pk_cost);
				r->rt_type = CHBRIDGE;
				r->rt_addr = pkt->pk_saddr;
			}
			break;
		case CHFIXED:
			break;
		default:
			debugf(DBG_WARN,
			       "CHAOS: Illegal chaos routing table entry %d",
			       r->rt_type);
		}
	}
	ch_free_pkt(pkt);
}

/*
 * Process a received RFC/BRD
 */
void
rcvrfc(struct packet *pkt)
{
	struct connection *conn, **conptr;
	struct packet **opkt, *pktl;

	if (1) {
		prpkt(pkt,"RFC/BRD");
		debugf(DBG_LOW, "contact = %s", pkt->pk_cdata);
		extern int flag_debug_level;
		if (flag_debug_level > 2) {
			dumpbuffer((u_char *)pkt, 64);
		}
	}

	/*
	 * Check if this is a duplicate RFC, and if so throw it away,
	 * and retransmit the OPEN.
	 */
	for (conptr = &Chconntab[0]; conptr < &Chconntab[CHNCONNS];)
		if ((conn = *conptr++) != NOCONN &&
		    CH_INDEX_SHORT(conn->cn_Fidx) == 
		    CH_INDEX_SHORT(pkt->pk_sidx) &&
		    CH_ADDR_SHORT(conn->cn_faddr) == 
		    CH_ADDR_SHORT(pkt->pk_saddr))
		{
			if (conn->cn_state == CSOPEN) {
				debugf(DBG_LOW,
				       "rcvrfc: Retransmitting open chan #%x",
				       CH_INDEX_SHORT(conn->cn_Lidx));
				if (conn->cn_thead != NOPKT)
					chretran(conn, CHSHORTTIME);
			} else {
				debugf(DBG_LOW, "rcvrfc: Duplicate RFC: %x",
				       CH_INDEX_SHORT(conn->cn_Lidx));
			}
			ch_free_pkt(pkt);
			return;
		}

	/*
	 * Scan the listen list for a listener and if one is found
	 * open the connection and remove the listen packet from the
	 * listen list.
	 */
	for (opkt = &Chlsnlist; (pktl = *opkt) != NOPKT; 
	     opkt = &pktl->pk_next)
	  if(concmp(pkt, pktl->pk_cdata, PH_LEN(pktl->pk_phead))) {
	    conn = Chconntab[pktl->pk_stindex];
	    *opkt = pktl->pk_next;
			ch_free_pkt(pktl);
			lsnmatch(pkt, conn);
			NEWSTATE(conn);
			return;
		}
	if (concmp(pkt, "STATUS", 6)) {
		statusrfc(pkt);
		return;
	}
	if (concmp(pkt, "TIME", 4)) {
		timerfc(pkt);
		return;
	}
	if (concmp(pkt, "UPTIME", 6)) {
		uptimerfc(pkt);
		return;
	}
	if (concmp(pkt, "DUMP-ROUTING-TABLE", 18)) {
		dumprtrfc(pkt);
		return;
	}
#if 1
	if (concmp(pkt, "FILE", 4)) {
		/* this is a hack to fake what chserver.c would do */
		struct packet *p = ch_alloc_pkt(10);
		strcpy(p->pk_cdata, "FILE");
		conn = ch_listen(p, 0);
		lsnmatch(pkt, conn);
		ch_accept(conn);

		start_file(conn, pkt->pk_cdata, PH_LEN(pkt->pk_phead));
		return;
	}
	if (concmp(pkt, "MINI", 4)) {
		/* this is a hack to fake what chserver.c would do */
		struct packet *p = ch_alloc_pkt(10);
		strcpy(p->pk_cdata, "MINI");
		conn = ch_listen(p, 0);
		lsnmatch(pkt, conn);
		ch_accept(conn);

		start_mini(conn, pkt->pk_cdata, PH_LEN(pkt->pk_phead));
		return;
	}
#endif
	if (Chrfcrcv == 0) {
		if (pkt->pk_op != BRDOP)
			send_los(pkt, "All servers disabled");
		return;
	}

	/*
	 * There was no listener, so queue the RFC on the unmatched RFC list
	 * again checking for duplicates.
	 */
	pkt->LE_pk_ackn = 0;	/* this is for ch_rskip */
	pkt->pk_next = NOPKT;
	if ((pktl = Chrfclist) == NOPKT) 
		Chrfclist = Chrfctail = pkt;
	else {
		do {
			if (CH_INDEX_SHORT(pktl->pk_sidx) == 
			    CH_INDEX_SHORT(pkt->pk_sidx) &&
			    CH_ADDR_SHORT(pktl->pk_saddr) == 
			    CH_ADDR_SHORT(pkt->pk_saddr)) {
				debugf(DBG_LOW,
				       "rcvrfc: Discarding duplicate Rfc on Chrfclist");
				ch_free_pkt(pkt);
				return;
			}
		} while ((pktl = pktl->pk_next) != NOPKT);
		Chrfctail->pk_next = pkt;
		Chrfctail = pkt;
	}

	debugf(DBG_INFO, "rcvrfc: Queued Rfc on Chrfclist: '%c%c%c%c'",
		pkt->pk_cdata[0], pkt->pk_cdata[1], pkt->pk_cdata[2],
		pkt->pk_cdata[3]);

	RFCINPUT;
}

/*
 * Process a received BRD
 */
void
rcvbrd(struct packet *pkt)
{
	int bitlen = LE_TO_SHORT(pkt->LE_pk_ackn);
	
	SET_CH_ADDR(pkt->pk_daddr,Chmyaddr);
	chmove(&pkt->pk_cdata[bitlen], pkt->pk_cdata, bitlen);
	SET_PH_LEN(pkt->pk_phead,PH_LEN(pkt->pk_phead)-bitlen);
	pkt->LE_pk_ackn = 0;
	rcvrfc(pkt);
}

/*
 * Process a received data packet - or an EOF packet which is mostly treated
 * the same.
 */
void
rcvdata(struct connection *conn, struct packet *pkt)
{
	register struct packet *npkt;

	if (1) {
		prpkt(pkt,"DATA");
		debugf(DBG_LOW, "\n");
	}

	if (cmp_gt(LE_TO_SHORT(pkt->LE_pk_pkn), 
		   conn->cn_rread + conn->cn_rwsize)) {
		debugf(DBG_WARN, "rcvdata: Discarding data out of window");
		debugf(DBG_WARN, "pkt->pkn is %x conn->cn_rread %x",
		       LE_TO_SHORT(pkt->LE_pk_pkn), conn->cn_rread);
		debugf(DBG_WARN, "conn->cn_rwsize is %x sum %x",
		       conn->cn_rread, conn->cn_rread + conn->cn_rwsize);
		ch_free_pkt(pkt);
		return;
	}

	receipt(conn, LE_TO_SHORT(pkt->LE_pk_ackn), 
		LE_TO_SHORT(pkt->LE_pk_ackn));

	if (cmp_le(LE_TO_SHORT(pkt->LE_pk_pkn), conn->cn_rlast)) {
		debugf(DBG_WARN, "rcvdata: Duplicate data packet");
		makests(conn, pkt);
		reflect(pkt);
		return;
	}

	/*
	 * Link the out of order list onto the new packet in case
	 * it fills the gap between in-order and out-of-order lists
	 * and to make it easy to grab all now-in-order packets from the
	 * out-of-order list.
	 */
	pkt->pk_next = conn->cn_routorder;

	/*
	 * Now transfer all in-order packets to the in-order list
	 */
	for (npkt = pkt;
	     npkt != NOPKT &&
	     	LE_TO_SHORT(npkt->LE_pk_pkn)
	       == (unsigned short)(conn->cn_rlast + 1);
	     npkt = npkt->pk_next) {
		if (conn->cn_rhead == NOPKT)
			conn->cn_rhead = npkt;
		else
			conn->cn_rtail->pk_next = npkt;
		conn->cn_rtail = npkt;
		conn->cn_rlast++;
	}

	/*
	 * If we queued up any in-order pkts, check if spontaneous STS is needed
	 */
	if (pkt != npkt) {
		debugf(DBG_INFO, "rcvdata: new ordered data packet");
		conn->cn_rtail->pk_next = NOPKT;
		conn->cn_routorder = npkt;
		if (conn->cn_rhead == pkt)
			INPUT(conn);

		/*
		 * If we are buffering "many" packets, keep him up to date
		 * about the window. The second test can be flushed if it is
		 * more important to receipt than to save sending an "extra"
		 * STS with no new acknowledgement.
		 * Lets do without it for a while.
		if ((short)(conn->cn_rlast - conn->cn_racked) > conn->cn_rsts &&
		    conn->cn_racked != conn->cn_rread)
			sendsts(conn);
		 */
	/*
	 * Here we have received an out of order packet which must be
	 * inserted into the out-of-order queue, in packet number order. 
	 */
	} else {
		for (npkt = pkt; (npkt = npkt->pk_next) != NOPKT &&
				 cmp_gt(LE_TO_SHORT(pkt->LE_pk_pkn), 
					LE_TO_SHORT(npkt->LE_pk_pkn)); )

			pkt->pk_next = npkt;	/* save the last pkt here */

		if (npkt != NOPKT && LE_TO_SHORT(pkt->LE_pk_pkn) == 
		    LE_TO_SHORT(npkt->LE_pk_pkn)) {
			debugf(DBG_INFO,
			       "rcvdata: Duplicate out of order packet");
			pkt->pk_next = NOPKT;
			makests(conn, pkt);
			reflect(pkt);
		} else {
			if (npkt == conn->cn_routorder)
				conn->cn_routorder = pkt;
			else
				pkt->pk_next->pk_next = pkt;

			pkt->pk_next = npkt;

			debugf(DBG_INFO, "rcvdata: New out of order packet %d", LE_TO_SHORT(pkt->LE_pk_pkn));
		}
	}
}


/*
 * Receive a packet - called from receiver interrupt level.
 */
void
rcvpkt(struct chxcvr *xp)
{
	int i;
	int fwdcnt;
	struct packet *pkt = (struct packet *)xp->xc_rpkt;
	struct connection *conn;
	unsigned index;

	tracef(TRACE_LOW, "rcvpkt:");
	
	/* xp->xc_rcvd++ */
	xp->LELNG_xc_rcvd = LE_TO_LONG(LE_TO_LONG(xp->LELNG_xc_rcvd) + 1);
	pkt->pk_next = NOPKT;
	if (CH_ADDR_SHORT(xp->xc_addr)) {
		struct chroute *r;
			
		r = &Chroutetab[xp->xc_subnet];
		r->rt_type = CHDIRECT;
		r->rt_cost = xp->xc_cost;
		r->rt_xcvr = xp;
	}

	debugf(DBG_LOW,
	       "rcvpkt: pkt->pk_daddr %o, Chmyaddr %o, xp->xc_addr %o",
	       CH_ADDR_SHORT(pkt->pk_daddr), Chmyaddr, 
	       CH_ADDR_SHORT(xp->xc_addr));

	if (CH_ADDR_SHORT(pkt->pk_daddr) != Chmyaddr &&
	    CH_ADDR_SHORT(pkt->pk_daddr) != CH_ADDR_SHORT(xp->xc_addr) &&
	    CH_ADDR_SHORT(pkt->pk_daddr) != 0) {
		
 
/* JAO: forwarding doesn't make sense unless we are a bridge, and have another subnet to transmit on. 
   As it is, this simply makes a lot of noise duplicating packets on the subnet that already have a
   receiver ready to receive them. */
   
	  /* increment forwarding count in packet */
	  fwdcnt = PH_FC(pkt->pk_phead);
	  SET_PH_FC(pkt->pk_phead,fwdcnt+1);
		if (((fwdcnt+1) & 0x0f) == 0) {
			debugf(DBG_WARN, "rcvpkt: Overforwarded packet");
ignore:
			ch_free_pkt(pkt);
		} else if (CH_ADDR_SHORT(pkt->pk_saddr) == Chmyaddr ||
			   CH_ADDR_SHORT(pkt->pk_saddr) == 
			   CH_ADDR_SHORT(xp->xc_addr)) {
			debugf(DBG_LOW, "rcvpkt: Got my own packet back");
			ch_free_pkt(pkt);
		} else if (Chmyaddr == -1)
			ch_free_pkt(pkt);
		else {
			debugf(DBG_LOW, "rcvpkt: NOT Forwarding pkt daddr=%x", /* JAO */
			       CH_ADDR_SHORT(pkt->pk_daddr));
#if(0) /* JAO */
			sendctl(pkt);
#endif 
		}
	}
	else if (pkt->pk_op == RUTOP)
		rcvrut(pkt);
	else if (pkt->pk_op == MNTOP)
		goto ignore;
	else if (pkt->pk_op == RFCOP) {
	  debugf(DBG_LOW, "identified RFC op");
		rcvrfc(pkt);
	}
	else if (pkt->pk_op == BRDOP)
		rcvbrd(pkt);

	/*
	 * Check for various flavors of bad indexes
	 */
	else if ((index = pkt->pk_dtindex) >= CHNCONNS ||
		 ((conn = Chconntab[index]) == NOCONN) ||
		 CH_INDEX_SHORT(conn->cn_Lidx) != CH_INDEX_SHORT(pkt->pk_didx))
	{
		debugf(DBG_WARN, "rcvpkt: Packet with bad index: %x, op:%d",
		       CH_INDEX_SHORT(pkt->pk_didx), pkt->pk_op);
		send_los(pkt, "Connection doesn't exist");

	/*
	 * Handle responses to our RFC
	 */
	} else if (conn->cn_state == CSRFCSENT)
		switch(pkt->pk_op) {
		case OPNOP:
			debugf(DBG_INFO,
			       "rcvpkt: Conn #%x: OPEN received",
			       CH_INDEX_SHORT(conn->cn_Lidx));

			/*
			 * Make the connection open, taking his index
			 */
			conn->cn_state = CSOPEN;
			conn->cn_Fidx = pkt->pk_sidx;
			conn->cn_active = Chclock;
			/*
			 * Indicate we have received AND "read" the OPEN
			 * Read his window size, indicate that he has
			 * received and acked the RFC, and acknowledge the OPN
			 */
			conn->cn_rread = conn->cn_rlast = 
			  LE_TO_SHORT(pkt->LE_pk_pkn);
			conn->cn_twsize = LE_TO_SHORT(pkt->LE_pk_rwsize);
			receipt(conn, LE_TO_SHORT(pkt->LE_pk_ackn), 
				LE_TO_SHORT(pkt->LE_pk_ackn));
			/*
			 * Wakeup the user waiting for the connection to open.
			 */
			makests(conn, pkt);
			reflect(pkt);
			NEWSTATE(conn);
			break;
		case CLSOP:
		case ANSOP:
		case FWDOP:
			debugf(DBG_LOW,
			       "rcvpkt: Conn #%x: CLOSE/ANS received for RFC",
			       CH_INDEX_SHORT(conn->cn_Lidx));
			clsconn(conn, CSCLOSED, pkt);
			break;
		default:
			debugf(DBG_LOW,
			       "rcvpkt: bad packet type "
			       "for conn #%x in CSRFCSENT: %d",
			       CH_INDEX_SHORT(conn->cn_Lidx), pkt->pk_op);
			send_los(pkt, "Bad packet type reponse to RFC");
		}
	/*
	 * Process a packet for an open connection
	 */
	else if (conn->cn_state == CSOPEN) {
		debugf(DBG_LOW, "rcvpkt: conn #%x open", 
		       CH_INDEX_SHORT(conn->cn_Lidx));
		conn->cn_active = Chclock;
		if (ISDATOP(pkt)) {
			debugf(DBG_LOW,
			       "rcvpkt: conn #%x open, giving it data",
			       CH_INDEX_SHORT(conn->cn_Lidx));
			rcvdata(conn, pkt);
		}
		else switch (pkt->pk_op) {
	    	case OPNOP:
			/*
			 * Ignore duplicate opens.
			 */
			debugf(DBG_LOW, "rcvpkt: Duplicate open received");
			ch_free_pkt(pkt);
			break;
		case SNSOP:
			if (1) prpkt(pkt, "SNS");
			receipt(conn, LE_TO_SHORT(pkt->LE_pk_ackn), 
				LE_TO_SHORT(pkt->LE_pk_ackn));
			makests(conn, pkt);
			reflect(pkt);
			break;
	    	case EOFOP:
			rcvdata(conn, pkt);
			break;
	    	case LOSOP:
		case CLSOP:
			debugf(DBG_LOW,
			       "rcvpkt: Close rcvd on %x", 
			       CH_INDEX_SHORT(conn->cn_Lidx));
			clsconn(conn, pkt->pk_op == CLSOP ? CSCLOSED : CSLOST, pkt);
			break;
			/*
			 * Uncontrolled data - que it at the head of the rlist
			 */
	    	case UNCOP:
			if ((pkt->pk_next = conn->cn_rhead) == NOPKT)
				conn->cn_rtail = pkt;
			conn->cn_rhead = pkt;
			if (pkt == conn->cn_rtail)
				INPUT(conn);
			receipt(conn, LE_TO_SHORT(pkt->LE_pk_ackn), 
				LE_TO_SHORT(pkt->LE_pk_ackn));
			break;
	    	case STSOP:
#if 1
			prpkt(pkt, "STS");
			debugf(DBG_INFO, 
			       "Conn #%x, Receipt=%d, Trans Window=%d",
			       conn->cn_Lidx,
			       LE_TO_SHORT(pkt->pk_idata[0]), 
			       LE_TO_SHORT(pkt->pk_idata[1]));
#endif
			if (LE_TO_SHORT(pkt->LE_pk_rwsize) > conn->cn_twsize)
				OUTPUT(conn);
			conn->cn_twsize = LE_TO_SHORT(pkt->LE_pk_rwsize);
			receipt(conn, LE_TO_SHORT(pkt->LE_pk_ackn), 
				LE_TO_SHORT(pkt->LE_pk_receipt));
			ch_free_pkt(pkt);
			if (conn->cn_thead != NOPKT)
				chretran(conn, CHSHORTTIME);
			break;
		default:
			debugf(DBG_LOW, "rcvpkt: bad opcode:%d", pkt->pk_op);
			send_los(pkt, "Bad opcode");
			/* should we do clsconn here? */
		}
	/*
	 * Connection is neither waiting for an OPEN nor OPEN.
	 */
	} else {
		debugf(DBG_LOW,
		       "rcvpkt: Packet for conn #%x (not open) "
		       "state=%d, op:%d",
		       CH_INDEX_SHORT(conn->cn_Lidx), 
		       conn->cn_state, pkt->pk_op);
		send_los(pkt, "Connection is closed");
	}
	debugf(DBG_LOW, "rcvpkt: at end");
}


/* ------------------------------------------------------------------------- */

#define E2BIG	10
#define ENOBUFS	11
#define ENXIO	12
#define EIO	13

/*
 * Return a connection or return NULL and set *errnop to any error.
 */
struct connection *
chopen(struct chopen *c, int wflag,int *errnop)
{
	struct connection *conn;
	struct packet *pkt;
	int rwsize, length;
        struct chopen cho;

        tracef(TRACE_LOW, "chopen(wflag=%d)", wflag);

	length = c->co_clength + c->co_length + (c->co_length ? 1 : 0);
	if (length > CHMAXPKT ||
	    c->co_clength <= 0) {
		*errnop = -E2BIG;
		return NOCONN;
	}

	debugf(DBG_LOW,
	       "chopen: c->co_length %d, c->co_clength %d, length %d",
	       c->co_length, c->co_clength, length);

	pkt = ch_alloc_pkt(length);
	if (pkt == NOPKT) {
		*errnop = -ENOBUFS;
		return NOCONN;
	}

	if (c->co_length)
		pkt->pk_cdata[c->co_clength] = ' ';

	memcpy(pkt->pk_cdata, c->co_contact, c->co_clength);
	if (c->co_length)
		memcpy(&pkt->pk_cdata[c->co_clength + 1], c->co_data,
		       c->co_length);

	rwsize = c->co_rwsize ? c->co_rwsize : CHDRWSIZE;
	SET_PH_LEN(pkt->pk_phead,length);

	conn = c->co_host ?
		ch_open(c->co_host, rwsize, pkt) :
		ch_listen(pkt, rwsize);

	if (conn == NOCONN) {
		debugf(DBG_LOW, "chopen: NOCONN");
		*errnop = -ENXIO;
		return NOCONN;
	}

	debugf(DBG_LOW, "chopen: c->co_async %d", c->co_async);
	debugf(DBG_LOW, "chopen: conn %p", conn);

#if 0
	if (!c->co_async) {
		/*
		 * We should hang until the connection changes from
		 * its initial state.
		 * If interrupted, flush the connection.
		 */

//		current->timeout = (unsigned long) -1;

		*errnop = chwaitfornotstate(conn, c->co_host ?
                                            CSRFCSENT : CSLISTEN);
		if (*errnop) {
			rlsconn(conn);
			return NOCONN;
		}

		/*
		 * If the connection is not open, the open failed.
		 * Unless is got an ANS back.
		 */
		if (conn->cn_state != CSOPEN &&
		    (conn->cn_state != CSCLOSED ||
		     (pkt = conn->cn_rhead) == NOPKT ||
		     pkt->pk_op != ANSOP))
		{
			debugf(DBG_LOW,
			       "chopen: open failed; cn_state %d",
			       conn->cn_state);
			rlsconn(conn);
			*errnop = -EIO;
			return NOCONN;
		}
	}
#endif

//	if (wflag)
//		conn->cn_sflags |= CHFWRITE;
//	conn->cn_sflags |= CHRAW;

	conn->cn_mode = CHSTREAM;
        debugf(DBG_LOW, "chopen() done");

	return conn;
}

struct chxcvr intf;

void
ch_rcv_pkt_buffer(char *buffer, int size)
{
	struct packet *pkt;
	struct chxcvr *xp;

	pkt = ch_alloc_pkt(size);
	if (pkt == 0)
		return;

	memcpy((char *)&pkt->pk_phead, buffer, size); 
	intf.xc_rpkt = pkt;

	rcvpkt(&intf);
}

extern int chaos_xmit(struct chxcvr *intf, struct packet *pkt, int at_head_p);

int
chaos_init(void)
{
	Chmyaddr = 0x0104;
	strcpy(Chmyname, "server");

	Chrfcrcv = 1;

	intf.xc_xmit = chaos_xmit;
	SET_CH_ADDR(intf.xc_addr,Chmyaddr);

	Chroutetab[1].rt_type = 0;

#if 1
	/* pick random uniq so a restarted server doesn't reuse old ids */
	srand(time(0L));
	uniq = rand();
#endif

	return 0;
}

// Userspace NCP protocol implementation

static struct packet *rfcseen;	/* used by ch_rnext and ch_listen */

struct packet *ch_alloc_pkt(int datasize);
void ch_close(struct connection *conn, struct packet *pkt, int release);

/*
 * Set packet fields from connection, many routines count on the fact that
 * this routine clears pk_type and pk_next
 */
void
setpkt(struct connection *conn, struct packet *pkt)
{
	pkt->pk_daddr = conn->cn_faddr;
	pkt->pk_didx = conn->cn_Fidx;
	pkt->pk_saddr = conn->cn_laddr;
	pkt->pk_sidx = conn->cn_Lidx;
	pkt->pk_type = 0;
	pkt->pk_next = NOPKT;
	SET_PH_FC(pkt->pk_phead,0);
}

int
ch_full(struct connection *conn)
{
    return chtfull(conn);
}

int
ch_empty(struct connection *conn)
{
    return chtempty(conn);
}

/* ------------------------------------------------------------------------ */

/*
 * Send a new data packet on a connection.
 * Called at high priority since window size check is elsewhere.
 */
int
ch_write(struct connection *conn, struct packet *pkt)
{
	tracef(TRACE_MED, "ch_write(pkt=%x)\n", pkt);

	switch (pkt->pk_op) {
	case ANSOP:
	case FWDOP:
		if (conn->cn_state != CSRFCRCVD ||
		    (conn->cn_flags & CHANSWER) == 0)
			goto err;
		ch_close(conn, pkt, 0);
		return 0;
	case RFCOP:
	case BRDOP:
		if (conn->cn_state != CSRFCSENT)
			goto err;
		break;
	case UNCOP:
		pkt->LE_pk_pkn = 0;
		setpkt(conn, pkt);
		senddata(pkt);
		return 0;
	default:
		if (!ISDATOP(pkt))
			goto err;
	case OPNOP:
	case EOFOP:
		if (conn->cn_state != CSOPEN)
			goto err;
		setack(conn, pkt);
		break;
	}
	setpkt(conn, pkt);
        ++conn->cn_tlast;
	pkt->LE_pk_pkn = LE_TO_SHORT(conn->cn_tlast);
	senddata(pkt);
	debugf(DBG_LOW, "ch_write: done");
	return 0;
err:
	ch_free_pkt(pkt);
	debugf(DBG_LOW, "ch_write: error");
	return CHERROR;
}

/*
 * Consume the packet at the head of the received packet queue (rhead).
 * Assumes high priority because check for available is elsewhere
 */
void
ch_read(struct connection *conn)
{
	struct packet *pkt;

        tracef(TRACE_MED, "ch_read:\n");
	if ((pkt = conn->cn_rhead) == NOPKT)
		return;
	conn->cn_rhead = pkt->pk_next;
	if (conn->cn_rtail == pkt)
		conn->cn_rtail = NOPKT;
	if (CONTPKT(pkt)) {
		conn->cn_rread = LE_TO_SHORT(pkt->LE_pk_pkn);
		if (pkt->pk_op == EOFOP ||
		    3 * (short)(conn->cn_rread - conn->cn_racked) > conn->cn_rwsize) {

			debugf(DBG_LOW,
                               "ch_read: Conn#%x: rread=%d rackd=%d rsts=%d\n",
			       conn->cn_Lidx, conn->cn_rread,
			       conn->cn_racked, conn->cn_rsts);

			pkt->pk_next = NOPKT;
			makests(conn, pkt);
			reflect(pkt);
			return;
		}
	}

	ch_free_pkt(pkt);
}

/*
 * Open a connection (send a RFC) given a destination host a RFC
 * packet, and a default receive window size.
 * Return the connection, on which the RFC has been sent.
 * The connection is not necessarily open at this point.
 */
struct connection *
ch_open(int destaddr, int rwsize, struct packet *pkt)
{
	struct connection *conn;

	tracef(TRACE_MED, "ch_open:\n");

	if ((conn = allconn()) == NOCONN) {
		ch_free_pkt(pkt);
		return(NOCONN);
	}
	/*
	 * This could be optimized to use the local address which has
	 * an active transmitter which is on the same subnet as the
	 * destination subnet.  This would involve knowing all the
	 * local addresses - which we don't currently maintain in a
	 * convenient form.
	 */
	SET_CH_ADDR(conn->cn_laddr,Chmyaddr);
	SET_CH_ADDR(conn->cn_faddr,destaddr);
	conn->cn_state = CSRFCSENT;
	SET_CH_INDEX(conn->cn_Fidx,0);
	conn->cn_rwsize = rwsize;
	conn->cn_rsts = rwsize / 2;
	conn->cn_active = Chclock;
	pkt->pk_op = RFCOP;
	SET_PH_FC(pkt->pk_phead,0);
	pkt->LE_pk_ackn = 0;

	debugf(DBG_LOW, "ch_open: Conn #%x: RFCS state\n", 
               CH_INDEX_SHORT(conn->cn_Lidx));

	/*
	 * By making the RFC packet written like a data packet,
	 * it will be freed by the normal receipting mechanism, enabling
	 * to easily be kept around for automatic retransmission.
	 * xmitdone (first half) and rcvpkt (handling OPEN pkts) help here.
	 * Since allconn clears the connection (including cn_tlast) the
	 * packet number of the RFC will be 1 (ch_write does pkn = ++tlast)
	 */

	ch_write(conn, pkt);	/* No errors possible */

	debugf(DBG_LOW, "ch_open: done\n");
	return(conn);
}

/*
 * Start a listener, given a packet with the contact name in it.
 * In all cases packet is consumed.
 * Connection returned is in the listen state.
 */
struct connection *
ch_listen(struct packet *pkt, int rwsize)
{
	struct connection *conn;
	struct packet *pktl, *opkt;

        tracef(TRACE_MED, "ch_listen()");

	if ((conn = allconn()) == NOCONN) {
		ch_free_pkt(pkt);
		return(NOCONN);
	}

	conn->cn_state = CSLISTEN;
	conn->cn_rwsize = rwsize;
	pkt->pk_op = LSNOP;
	setpkt(conn, pkt);

	opkt = NOPKT;
	for (pktl = Chrfclist; pktl != NOPKT; pktl = (opkt = pktl)->pk_next)
		if (concmp(pktl, pkt->pk_cdata, (int)(PH_LEN(pkt->pk_phead)))) {
			if(opkt == NOPKT)
				Chrfclist = pktl->pk_next;
			else
				opkt->pk_next = pktl->pk_next;

			if (pktl == Chrfctail)
				Chrfctail = opkt;
			if (pktl == rfcseen)
				rfcseen = NOPKT;

			ch_free_pkt(pkt);
			lsnmatch(pktl, conn);

			return(conn);
		}
	/*
	 * Should we check for duplicate listeners??
	 * Or is it better to allow more than one?
	 */
	pkt->pk_next = Chlsnlist;
	Chlsnlist = pkt;
	debugf(DBG_LOW, "ch_listen: Conn #%x: LISTEN state\n", 
               CH_INDEX_SHORT(conn->cn_Lidx));

	return(conn);
}

/*
 * Accept an RFC, called on a connection in the CSRFCRCVD state.
 */
void
ch_accept(struct connection *conn)
{
	struct packet *pkt = ch_alloc_pkt(sizeof(struct sts_data));

	tracef(TRACE_MED, "ch_accept: state %o, want %o\n",
               conn->cn_state, CSRFCRCVD);

	if (conn->cn_state != CSRFCRCVD)
		ch_free_pkt(pkt);
	else {
		if (conn->cn_rhead != NOPKT && conn->cn_rhead->pk_op == RFCOP)
			 ch_read(conn);
		conn->cn_state = CSOPEN;
		conn->cn_tlast = 0;
		conn->cn_rsts = conn->cn_rwsize >> 1;
		pkt->pk_op = OPNOP;
		SET_PH_LEN(pkt->pk_phead,sizeof(struct sts_data));
		pkt->LE_pk_receipt = LE_TO_SHORT(conn->cn_rlast);
		pkt->LE_pk_rwsize = LE_TO_SHORT(conn->cn_rwsize);

		debugf(DBG_LOW, "ch_accept: Conn #%o: open sent\n",
                       CH_INDEX_SHORT(conn->cn_Lidx));

		ch_write(conn, pkt);
	}
}

/*
 * Close a connection, giving close pkt to send (CLS or ANS).
 */
void
ch_close(struct connection *conn, struct packet *pkt, int release)
{
        tracef(TRACE_MED, "ch_close()\n");

	switch (conn->cn_state) {
	    case CSOPEN:
	    case CSRFCRCVD:
		if (pkt != NOPKT) {
			setpkt(conn, pkt);
			pkt->LE_pk_ackn = pkt->LE_pk_pkn = 0;
			sendctl(pkt);
			pkt = NOPKT;
		}
		/* Fall into... */
	    case CSRFCSENT:
		clsconn(conn, CSCLOSED, NOPKT);
		break;
	    case CSLISTEN:
		rmlisten(conn);
		break;
	    default:
		break;
	}
	if (pkt)
		ch_free_pkt(pkt);

	if (release)
		rlsconn(conn);
}

int
ch_setmode(struct connection *conn, int mode)
{
    int ret = 0;

    switch (mode) {
    case CHTTY:
        ret = -1;
        break;
    case CHSTREAM:
    case CHRECORD:
        if (conn->cn_mode == CHTTY)
            ret = -1;
        else
            conn->cn_mode = mode;
    }

    return ret;
}

/*
 * process a RFC for contact name STATUS
 */
void
statusrfc(struct packet *pkt)
{
	struct chroute *r;
	struct chxcvr *xp;
	struct statdata *sp;
	int i;
	int saddr = CH_ADDR_SHORT(pkt->pk_saddr);
	int sidx = CH_INDEX_SHORT(pkt->pk_sidx);
	int daddr = CH_ADDR_SHORT(pkt->pk_daddr);
	
	debugf(DBG_LOW, "statusrfc:");

	ch_free_pkt(pkt);
	for (i = 0, r = Chroutetab; r < &Chroutetab[CHNSUBNET]; r++)
		if (r->rt_type == CHDIRECT)
			i++;
	i *= sizeof(struct stathead) + sizeof(struct statxcvr);
	i += CHSTATNAME;
	if ((pkt = ch_alloc_pkt(i)) == NOPKT)
		return;
	SET_CH_ADDR(pkt->pk_daddr,saddr);
	SET_CH_INDEX(pkt->pk_didx,sidx);
	pkt->pk_type = 0;
	pkt->pk_op = ANSOP;
	pkt->pk_next = NOPKT;
	SET_CH_ADDR(pkt->pk_saddr,daddr);
	SET_CH_INDEX(pkt->pk_sidx,0);
	pkt->LE_pk_pkn = pkt->LE_pk_ackn = 0;
	SET_PH_LEN(pkt->pk_phead,i); 	  /* pkt->pk_lenword = i; */

	chmove(Chmyname, pkt->pk_status.sb_name, CHSTATNAME);
	sp = &pkt->pk_status.sb_data[0];
	for (r = Chroutetab; r < &Chroutetab[CHNSUBNET]; r++) {
		if (r->rt_type == CHDIRECT) {
			xp = r->rt_xcvr;
			sp->LE_sb_ident = LE_TO_SHORT(0400 + xp->xc_subnet);
			sp->LE_sb_nshorts = 
			  LE_TO_SHORT(sizeof(struct statxcvr) / sizeof(short));
			sp->sb_xstat = xp->xc_xstat;
			sp = (struct statdata *)((char *)sp +
				sizeof(struct stathead) +
				sizeof(struct statxcvr));
		}
	}

	debugf(DBG_LOW, "statusrfc: answering");

	sendctl(pkt);
}

/*
 * Return the time according to the chaos TIME protocol, in a long.
 * No byte shuffling need be done here, just time conversion.
 */
void
ch_time(long *tp)
{
	struct timeval time;

	gettimeofday(&time, NULL);

	*tp = time.tv_sec;
	*tp += 60UL*60*24*((1970-1900)*365L + 1970/4 - 1900/4);
}

void
ch_uptime(long *tp)
{
//	*tp = jiffies;
	*tp = 0;
}

/*
 * process a RFC for contact name TIME 
 */
void
timerfc(struct packet *pkt)
{
	long t;

	debugf(DBG_LOW, "timerfc: answering");

	pkt->pk_op = ANSOP;
	pkt->pk_next = NOPKT;
	pkt->LE_pk_pkn = pkt->LE_pk_ackn = 0;
	SET_PH_LEN(pkt->pk_phead, sizeof(long));
	ch_time(&t);
	pkt->pk_ldata[0] = LE_TO_LONG(t);
	reflect(pkt);
}

void
uptimerfc(struct packet *pkt)
{
	long t;

	pkt->pk_op = ANSOP;
	pkt->pk_next = NOPKT;
	pkt->LE_pk_pkn = pkt->LE_pk_ackn = 0;
	SET_PH_LEN(pkt->pk_phead, sizeof(long));
	ch_uptime(&t);
	pkt->pk_ldata[0] = LE_TO_LONG(t);
	reflect(pkt);
}

void
dumprtrfc(struct packet *pkt)
{
	struct chroute *r;
	short *wp;
	int ndirect, i;
	int saddr = CH_ADDR_SHORT(pkt->pk_saddr);
	int sidx = CH_INDEX_SHORT(pkt->pk_sidx);
	int daddr = CH_ADDR_SHORT(pkt->pk_daddr);
	
	ch_free_pkt(pkt);
	if ((pkt = ch_alloc_pkt(CHNSUBNET * 4)) != NOPKT) {
		wp = pkt->pk_idata;
		ndirect = i = 0;
		for (r = Chroutetab; r < &Chroutetab[CHNSUBNET]; r++, i++)
			if (r->rt_type == CHDIRECT) {
				*wp++ = (ndirect++ << 1) + 1;
				*wp++ = i;
			} else {
				*wp++ = CH_ADDR_SHORT(r->rt_addr);
				*wp++ = r->rt_cost;
			}
		SET_CH_ADDR(pkt->pk_daddr,saddr);
		SET_CH_INDEX(pkt->pk_didx,sidx);
		pkt->pk_type = 0;
		pkt->pk_op = ANSOP;
		pkt->pk_next = NOPKT;
		SET_CH_ADDR(pkt->pk_saddr,daddr);
		SET_CH_INDEX(pkt->pk_sidx,0);
		pkt->LE_pk_pkn = pkt->LE_pk_ackn = 0;
		SET_PH_LEN(pkt->pk_phead,CHNSUBNET * 4);
		sendctl(pkt);
	}
}
