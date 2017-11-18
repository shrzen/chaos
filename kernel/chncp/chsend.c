#include "../h/chaos.h"
#include "../chunix/chsys.h"
#include "../chunix/chconf.h"
#include "chncp.h"

#include "../chunix/chlinux.h"

#ifdef linux
#define printf printk
#endif

/*
 * Send a STS packet on this connection
 *	if allocation fails it is not sent
 */
void
sendsts(register struct connection *conn)
{
	register struct packet *pkt;

	if ((pkt = pkalloc(sizeof(struct sts_data), 1)) != NOPKT) {
		setpkt(conn, pkt);
		makests(conn, pkt);
		sendctl(pkt);
	}
}

/*
 * Send a SNS packet on this connection
 *	if allocation failed nothing is sent
 */
void
sendsns(register struct connection *conn)
{
	register struct packet *pkt;

	if ((pkt = pkalloc(0, 1)) != NOPKT) {
		setpkt(conn, pkt);
		pkt->pk_op = SNSOP;
		SET_PH_LEN(pkt->pk_phead, 0);
		setack(conn, pkt);
		sendctl(pkt);
	}
}

int
chxmitpkt(register struct chxcvr *xcvr, register struct packet *pkt, int head)
{
	register struct packet *tpkt;

	if (head) {
		pkt->pk_next = xcvr->xc_list;

if (pkt->pk_next == pkt)
{ showpkt("chxmitpkt",pkt);
#if 0
  panic("chxmitpkt: pkt->pk_next = pkt");
#endif
}

		xcvr->xc_list = pkt;
		if (xcvr->xc_tail == NOPKT)
			xcvr->xc_tail = pkt;
	} else {
		pkt->pk_next = NOPKT;
		if ((tpkt = xcvr->xc_tail) != NOPKT)
{
			tpkt->pk_next = pkt;

if (tpkt->pk_next == tpkt)
{ showpkt("chxmitpkt",tpkt);
#if 0
  panic("chxmitpkt: tpkt->pk_next = tpkt");
#endif
}
}

		else
			xcvr->xc_list = pkt;
		xcvr->xc_tail = pkt;
	}
	if (xcvr->xc_tpkt == NOPKT)
		(*xcvr->xc_start)(xcvr);
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

	debug(DSEND, (printf("Sending: %d ", pkt->pk_op), prpkt(pkt, "ctl"), printf("\n")));
	if (CH_ADDR_SHORT(pkt->pk_daddr) == Chmyaddr) {
		debug(DPKT,printf("to me\n"));
		sendtome(pkt);
	} else if (pkt->pk_dsubnet >= CHNSUBNET ||
	    (r = &Chroutetab[pkt->pk_dsubnet])->rt_type == CHNOPATH ||
	     r->rt_cost >= CHHCOST) {
		debug(DSEND, printf("chaos: Dropping, no route"));
		ch_free(pkt);
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

	debug(DSEND, (printf("Sending: %d ", pkt->pk_op),
                      prpkt(pkt, "data"), printf("\n")));
	if (CH_ADDR_SHORT(pkt->pk_daddr) == Chmyaddr) {
		debug(DPKT,printf("to me\n"));
		sendtome(pkt);
        }
	else if (pkt->pk_dsubnet >= CHNSUBNET ||
	    (r = &Chroutetab[pkt->pk_dsubnet])->rt_type == CHNOPATH ||
	     r->rt_cost >= CHHCOST) {
		struct packet *npkt;
		debug(DPKT|DABNOR,printf("no path to 0%x\n", CH_ADDR_SHORT(pkt->pk_daddr)))
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
			SET_CH_ADDR(pkt->pk_xdest, dest);
			next = pkt->pk_next;
			(*xcvr->xc_xmit)(xcvr, pkt, 0);
			if ((pkt = next) == NOPKT)
				break;
		}
	}
}
/*
 * Send the given RUT packet out on the given tranceiver, which has the given
 * cost.  If the "copy" flag is true, make a copy of the packet.
 * Note that if copy is not set, the packet data gets modified.
 */
void
sendrut(register struct packet *pkt,register struct chxcvr *xcvr,unsigned short cost,int copy)
{
	register struct rut_data *rd;
	struct rut_data *rdend;

	if (copy) {
		struct packet *npkt;

		if ((npkt = pkalloc((int)PH_LEN(pkt->pk_phead), 1)) == NOPKT)
			return;
		movepkt(pkt, npkt);
		pkt = npkt;
	}
	rdend = (struct rut_data *)(pkt->pk_cdata + PH_LEN(pkt->pk_phead));
	for (rd = pkt->pk_rutdata; rd < rdend; rd++)
		rd->LE_pk_cost += cost;
	pkt->pk_saddr = xcvr->xc_addr;
	SET_CH_ADDR(pkt->pk_xdest, 0);
	(*xcvr->xc_xmit)(xcvr, pkt, 1);
}
/*
 * Send the (list of) packet(s) to myself - NOTE THIS CAN BE RECURSIVE!
 */
void
sendtome(register struct packet *pkt)
{
	struct packet *rpkt, *npkt;
	static struct chxcvr fakexcvr;

	debug(DPKT,printf("sendtome()\n"));
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
			if ((rpkt = pkalloc(PH_LEN(pkt->pk_phead), 1)) != NOPKT)
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
 * Indicate that actual transmission of the current packet has been completed.
 * Called by the device dependent interrupt routine when transmission
 * of a packet has finished.
 */
void
xmitdone(struct packet *pkt)
{
	struct connection *conn;
	struct packet *npkt;

#if 0
	/* SC - find out what priority we're running at */
	s = spl7();
	splx(s);
	if (s < 0x15)
	  {
	    printf("\nxmitdone: ipl %d\n",s);
	    showpkt("xmitdone",pkt);
	    printf("xmitdone at low priority");
	  }
#endif


	if (PH_FC(pkt->pk_phead) == 0 && CONTPKT(pkt) &&
	    pkt->pk_stindex < CHNCONNS &&
	    (conn = Chconntab[pkt->pk_stindex]) != NOCONN &&
	    CH_INDEX_SHORT(pkt->pk_sidx) == CH_INDEX_SHORT(conn->cn_Lidx) &&
	    (conn->cn_state == CSOPEN || conn->cn_state == CSRFCSENT) &&
	    cmp_gt(LE_TO_SHORT(pkt->LE_pk_pkn), conn->cn_trecvd)) {
		pkt->pk_time = Chclock;
		if ((npkt = conn->cn_thead) == NOPKT || 
		    cmp_lt(LE_TO_SHORT(pkt->LE_pk_pkn), LE_TO_SHORT(npkt->LE_pk_pkn))) {
			pkt->pk_next = npkt;

#if 0
if (pkt->pk_next == pkt)
{ showpkt("xmitdone",pkt);
  panic("xmitdone: pkt->pk_next = pkt");}
#endif

			conn->cn_thead = pkt;
		} else {
			for( ; npkt->pk_next != NOPKT; npkt = npkt->pk_next)
				if(cmp_lt(LE_TO_SHORT(pkt->LE_pk_pkn), LE_TO_SHORT(npkt->pk_next->LE_pk_pkn)))
					break;
			pkt->pk_next = npkt->pk_next;

#if 0
if (pkt->pk_next == pkt)
{ showpkt("xmitdone",pkt);
  panic("xmitdone:npkt->pk_next = pkt");}
#endif

			npkt->pk_next = pkt;

#if 0
if (npkt->pk_next == npkt)
{ showpkt("xmitdone", npkt);
  panic("xmitdone:npkt->pk_next = npkt");}
#endif

		}
		if(pkt->pk_next == NOPKT)
			conn->cn_ttail = pkt;
	} else
		ch_free(pkt);
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
	return (pkt);
}
