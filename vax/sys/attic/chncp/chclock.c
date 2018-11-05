/*
 *	$Source: /afs/dev.mit.edu/source/src77/bsd-4.3/vax/sys/attic/chncp/RCS/chclock.c,v $
 *	$Author: cfields $
 *	$Locker:  $
 *	$Log: chclock.c,v $
 * Revision 1.1  1994/09/16  11:06:25  cfields
 * Initial revision
 *
 * Revision 1.1  84/06/12  20:27:08  jis
 * Initial revision
 * 
 */
#ifndef lint
static char *rcsid_chclock_c = "$Header: /afs/dev.mit.edu/source/src77/bsd-4.3/vax/sys/attic/chncp/RCS/chclock.c,v 1.1 1994/09/16 11:06:25 cfields Exp $";
#endif lint
#include "../h/chaos.h"
#include "../chunix/chsys.h"
#include "../chunix/chconf.h"
#include "../chncp/chncp.h"

/*
 * Clock level processing.
 *	ch_clock should be called each clock tick (HZ per second)
 *	at a priority equal to or higher that LOCK.
 *
 * Terminology:
 *    Packet aging:	Retransmitting packets that are not acked within
 *			AGERATE ticks
 *
 *    Probe:		The sending of a SNS packet if not all of the packets
 *			we have sent have been acknowledged
 *
 *    Responding:	Send a SNS every so often to see if the guy is still
 *			alive (after NRESPONDS we declare him dead)
 *
 *    RFC aging:	The retransmission of RFC packets
 *
 *    Route aging:	Increase the cost of transmitting over gateways so we
 *			will use another gateway if the current gateway goes
 *			down.
 *    Route broadcast:	If we are connected to more than one subnet, broad
 *			cast our bridge status every BRIDGERATE seconds.
 *
 *    Interface hung:	Checking periodically for dead interfaces (or dead
 *			"other end"'s of point-to-point links).
 *
 * These rates might want to vary with the cost of getting to the host.
 * They also might want to reside in the chconf.h file if they are not a real
 * network standard.
 *
 * Since these rates are dependent on a run-time variable
 * (This is a good idea if you think about it long enough),
 * We might want to initialize specific variables at run-time to
 * avoid recalculation if the profile of chclock is disturbing.
 */
#define MINRATE		ORATE		/* Minimum of following rates */
#define HANGRATE	(Chhz>>1)	/* How often to check for hung
					   interfaces */
#define AGERATE		(Chhz)		/* Re-xmit pkt if not rcptd in time */
#define PROBERATE	(Chhz<<3)	/* Send SNS to get STS for receipts or
					   to make sure the conn. is alive */
#define ORATE		(Chhz>>1)	/* Xmit current (stream) output packet
					   if not touched in this time */
#define TESTRATE	(Chhz*45)	/* Respond testing rate */
#define ROUTERATE	(Chhz<<2)	/* Route aging rate */
#define BRIDGERATE	(Chhz*15)	/* Routing broadcast rate */
#define NRESPONDS	3		/* Test this many times before timing
					   out the connection */
#define UPTIME  	(NRESPONDS*TESTRATE)	/* Nothing in this time and
					   the connection is flushed */
#define RFCTIME		(Chhz*15)	/* Try CHRFCTRYS times to RFC */

chtime Chclock;
int	Chnobridge;

ch_clock()
{
	register struct connection *conn;
	register struct connection **connp;
	register struct packet *pkt;
	chtime inactive;
	int probing;			/* are we probing this time ? */
	static chtime nextclk = 1;	/* next time to do anything */
	static chtime nextprobe = 1;	/* next time to probe */
	static chtime nexthang = 1;	/* next time to chxtime() */
	static chtime nextroute = 1;	/* next time to age routing */
	static chtime nextbridge = 1;	/* next time to send routing */

	if (nextclk != ++Chclock)
		return;
	nextclk += MINRATE;
	if (cmp_gt(Chclock, nextprobe)) {
		probing = 1;
		nextprobe += PROBERATE;
	} else
		probing = 0;
	if (cmp_gt(Chclock, nexthang)) {
		chxtime();
		nexthang += HANGRATE;
	}
	if (cmp_gt(Chclock, nextroute)) {
		chroutage();
		nextroute += ROUTERATE;
	}
	if (cmp_gt(Chclock, nextbridge)) {
		chbridge();
		nextbridge += BRIDGERATE;
	}
	debug(DNOCLK,return);
	for (connp = &Chconntab[0]; connp < &Chconntab[CHNCONNS]; connp++) {
		if ((conn = *connp) == NOCONN ||
		    (conn->cn_state != CSOPEN &&
		     conn->cn_state != CSRFCSENT))
			continue;
#ifdef CHSTRCODE
		if ((pkt = conn->cn_toutput) != NOPKT &&
		    cmp_gt(Chclock, pkt->pk_time + ORATE) &&
		    !chtfull(conn)) {
			conn->cn_toutput = NOPKT;
			(void)ch_write(conn, pkt);
		}
#endif
		if (conn->cn_thead != NOPKT)
			chretran(conn, AGERATE);
		if (probing) {
			inactive = Chclock - conn->cn_active;
			if (inactive >= (conn->cn_state == CSOPEN ?
					 UPTIME : RFCTIME)) {
				debug(DCONN|DABNOR,
					printf("Conn #%x: Timeout\n",
						conn->cn_lidx));
				clsconn(conn, CSINCT, NOPKT);
			} else if (conn->cn_state == CSOPEN &&
				   (conn->cn_tacked != conn->cn_tlast ||
				    chtfull(conn) ||
				    inactive >= TESTRATE)) {
				debug(DCONN, printf("Conn #%x: Probe: %D\n",
						    conn->cn_lidx, inactive));
				sendsns(conn);
			}
		}
	}
}

int printretrans = 0;
chretran(conn, age)
struct connection *conn;
{
	register struct packet *pkt, **opkt;
	register struct packet *lastpkt;
	struct packet *firstpkt = NOPKT;

	for (opkt = &conn->cn_thead; pkt = *opkt;)
		if (cmp_gt(Chclock, pkt->pk_time + age)) {
			if (firstpkt == NOPKT) 
				firstpkt = pkt;
			else
  {
				lastpkt->pk_next = pkt;

if (lastpkt->pk_next == lastpkt) 
{showpkt("chretran",lastpkt);
 panic("chretran: lastpkt->pk_next = lastpkt");}

    }
			lastpkt = pkt;
			*opkt = pkt->pk_next;
			pkt->pk_next = NOPKT;
			setack(conn, pkt);
		} else
			opkt = &pkt->pk_next;
	if (firstpkt != NOPKT) {

if (printretrans)
     showpkt("chretran", firstpkt);

		debug(DCONN|DABNOR,
			printf("Conn #%x: Rexmit (op:%d, pkn:%d)\n",
				conn->cn_lidx, firstpkt->pk_op,
				firstpkt->pk_pkn));
		senddata(firstpkt);
	}
}
/*
 * Increase the cost of accessing a subnet via a gateway
 */
chroutage()
{
	register struct chroute *r;

	for (r = Chroutetab; r < &Chroutetab[CHNSUBNET]; r++)
		if ((r->rt_type == CHBRIDGE || r->rt_type == CHDIRECT) &&
		    r->rt_cost < CHHCOST)
			r->rt_cost++;
}
/*
 * Send routing packets on all directly connected subnets, unless we are on
 * only one.
 */
chbridge()
{
	register struct chroute *r;
	register struct packet *pkt;
	register struct rut_data *rd;
	register int ndirect;
	register int n;

	if (Chnobridge)
		return;
	/*
	 * Count the number of subnets to which we are directly connected.
	 * If not more than one, then we are not a bridge and shouldn't
	 * send out routing packets at all.
	 * While we're at it, count the number of subnets we know we
	 * have any access to.  This number determines the size of the
	 * routine packet we need to send, if any.
	 */
	n = ndirect = 0;
	for (r = Chroutetab; r < &Chroutetab[CHNSUBNET]; r++)
		if (r->rt_cost < CHHCOST)
			switch(r->rt_type) {
			case CHDIRECT:
				ndirect++;
			default:
				n++;
				break;
			case CHNOPATH:
				;	
			}
	if (ndirect <= 1 ||
	    (pkt = pkalloc(n * sizeof(struct rut_data), 1)) == NOPKT)
		return;
	/*
	 * Build the routing packet to send out on each directly connected
	 * subnet.  It is complete except for the cost of the directly
	 * connected subnet we are sending it out on.  This cost must be
	 * added to each entry in the packet each time it is sent.
	 */
	pkt->pk_len = n * sizeof(struct rut_data);
	pkt->pk_op = RUTOP;
	pkt->pk_type = pkt->pk_daddr = pkt->pk_sidx = pkt->pk_didx = 0;
	pkt->pk_next = NOPKT;
	rd = pkt->pk_rutdata;
	for (n = 0, r = Chroutetab; r < &Chroutetab[CHNSUBNET]; r++, n++)
		if (r->rt_cost < CHHCOST && r->rt_type != CHNOPATH) {
			rd->pk_subnet = n;
			rd->pk_cost = r->rt_cost;
			rd++;
		}
	/*
	 * Now send out this packet on each directly connected subnet.
	 * ndirect becomes zero on last such subnet.
	 */
	for (r = Chroutetab; r < &Chroutetab[CHNSUBNET]; r++)
		if (r->rt_type == CHDIRECT && r->rt_cost < CHHCOST)
			sendrut(pkt, r->rt_xcvr, r->rt_cost, --ndirect);
}
