#include "../h/chaos.h"
#include "chsys.h"
#include "../chunix/chconf.h"
#include "chncp.h"
/*#include "chip.h"*/

#if defined(linux) && defined(__KERNEL__)
#include "chlinux.h" 
#endif

#if defined(linux) && !defined(__KERNEL__)
#define panic(x) exit(127)
#endif

/*
 * Receive side - basically driven by an incoming packet.
 */

#define send_los(x,y) sendlos(x,y,sizeof(y) - 1)

/*
 * Receive a packet - called from receiver interrupt level.
 */
void
rcvpkt(struct chxcvr *xp)
{
	int fwdcnt;
	struct packet *pkt = (struct packet *)xp->xc_rpkt;
	struct connection *conn;
	unsigned index;

	debug(DPKT,printf("Rcvpkt: "));
	xp->LELNG_xc_rcvd = LE_TO_LONG(LE_TO_LONG(xp->LELNG_xc_rcvd) + 1);
	pkt->pk_next = NOPKT;
	if (CH_ADDR_SHORT(xp->xc_addr)) {
		struct chroute *r;
			
		r = &Chroutetab[xp->xc_subnet];
		r->rt_type = CHDIRECT;
		r->rt_cost = xp->xc_cost;
		r->rt_xcvr = xp;
	}

	debug(DPKT|DABNOR,printf(
	       "rcvpkt: pkt->pk_daddr %o, Chmyaddr %o, xp->xc_addr %o",
	       CH_ADDR_SHORT(pkt->pk_daddr), Chmyaddr, 
	       CH_ADDR_SHORT(xp->xc_addr)));

	if (CH_ADDR_SHORT(pkt->pk_daddr) != Chmyaddr &&
	    CH_ADDR_SHORT(pkt->pk_daddr) != CH_ADDR_SHORT(xp->xc_addr) &&
	    CH_ADDR_SHORT(pkt->pk_daddr) != 0) {
/* JAO: forwarding doesn't make sense unless we are a bridge, and have another subnet to transmit on. 
   As it is, this simply makes a lot of noise duplicating packets on the subnet that already have a
   receiver ready to receive them. */
   
		/* increment forwarding count in packet */
		fwdcnt = PH_FC(pkt->pk_phead);
		SET_PH_FC(pkt->pk_phead, fwdcnt + 1);
		if (((fwdcnt+1) & 0x0f) == 0) {
			debug(DPKT|DABNOR,printf("Overforwarded packet\n"));
ignore:
			ch_free((char*)pkt);
		} else if (CH_ADDR_SHORT(pkt->pk_saddr) == Chmyaddr ||
			   CH_ADDR_SHORT(pkt->pk_saddr) == CH_ADDR_SHORT(xp->xc_addr)) {
			debug(DPKT|DABNOR,printf("Got my own packet back\n"));
			ch_free((char*)pkt);
		} else if (Chmyaddr == -1)
			ch_free((char*)pkt);
		else {
			debug(DPKT,printf("(NOT) Forwarding pkt daddr=%x\n", CH_ADDR_SHORT(pkt->pk_daddr)));
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
		debug(DPKT,printf("identified RFC op"));
		rcvrfc(pkt);
	}
	else if (pkt->pk_op == BRDOP)
		rcvbrd(pkt);
#if NCHIP > 0
        else if (pkt->pk_op == UNCOP) {
		chipinput (pkt);
        }
#endif
	/*
	 * Check for various flavors of bad indexes
	 */
	else if ((index = pkt->pk_dtindex) >= CHNCONNS ||
		 ((conn = Chconntab[index]) == NOCONN) ||
		 CH_INDEX_SHORT(conn->cn_Lidx) != CH_INDEX_SHORT(pkt->pk_didx)) {
		debug(DPKT|DABNOR,printf("Packet with bad index: %x, op:%d\n",
			CH_INDEX_SHORT(pkt->pk_didx), pkt->pk_op));
		send_los(pkt, "Connection doesn't exist");
	/*
	 * Handle responses to our RFC
	 */
	} else if (conn->cn_state == CSRFCSENT)
		switch(pkt->pk_op) {
		case OPNOP:
			debug(DCONN,printf("Conn #%x: OPEN received\n", CH_INDEX_SHORT(conn->cn_Lidx)));
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
			conn->cn_rread = conn->cn_rlast = LE_TO_SHORT(pkt->LE_pk_pkn);
			conn->cn_twsize = LE_TO_SHORT(pkt->LE_pk_rwsize);
			receipt(conn, LE_TO_SHORT(pkt->LE_pk_ackn), LE_TO_SHORT(pkt->LE_pk_ackn));
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
			debug(DCONN,printf("Conn #%x: CLOSE/ANS received for RFC\n", CH_INDEX_SHORT(conn->cn_Lidx)));
			clsconn(conn, CSCLOSED, pkt);
			break;
		default:
			debug(DPKT|DABNOR,
				printf("bad packet type for conn #%x in CSRFCSENT: %d\n",
					CH_INDEX_SHORT(conn->cn_Lidx), pkt->pk_op));
			send_los(pkt, "Bad packet type reponse to RFC");
		}
	/*
	 * Process a packet for an open connection
	 */
	else if (conn->cn_state == CSOPEN) {
		debug(DPKT,printf("rcvpkt: conn #%x open", 
				  CH_INDEX_SHORT(conn->cn_Lidx)));
		conn->cn_active = Chclock;
		if (ISDATOP(pkt)) {
			debug(DPKT|DABNOR, printf(
			       "rcvpkt: conn #%x open, giving it data",
			       CH_INDEX_SHORT(conn->cn_Lidx)));
			rcvdata(conn, pkt);
		}
		else switch (pkt->pk_op) {
	    	case OPNOP:
			/*
			 * Ignore duplicate opens.
			 */
			debug(DPKT|DABNOR,printf("Duplicate open received\n"));
			ch_free((char*)pkt);
			break;
		case SNSOP:
			debug(DPKT,prpkt(pkt, "SNS"));
			receipt(conn, LE_TO_SHORT(pkt->LE_pk_ackn), LE_TO_SHORT(pkt->LE_pk_ackn));
			makests(conn, pkt);
			reflect(pkt);
			break;
	    	case EOFOP:
			rcvdata(conn, pkt);
			break;
	    	case LOSOP:
		case CLSOP:
			debug(DCONN, printf("Close rcvd on %x\n", CH_INDEX_SHORT(conn->cn_Lidx)));
			clsconn(conn, pkt->pk_op == CLSOP ? CSCLOSED : CSLOST,
				pkt);
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
			receipt(conn, LE_TO_SHORT(pkt->LE_pk_ackn), LE_TO_SHORT(pkt->LE_pk_ackn));
			break;
	    	case STSOP:
#if 1
			debug(DABNOR,prpkt(pkt, "STS"));
			debug(DABNOR,printf("Conn #%x, Receipt=%d, Trans Window=%d\n",
					    conn->cn_Lidx, LE_TO_SHORT(pkt->pk_idata[0]), LE_TO_SHORT(pkt->pk_idata[1])));
#endif
			if (LE_TO_SHORT(pkt->LE_pk_rwsize) > conn->cn_twsize)
				OUTPUT(conn);
			conn->cn_twsize = LE_TO_SHORT(pkt->LE_pk_rwsize);
			receipt(conn, LE_TO_SHORT(pkt->LE_pk_ackn), LE_TO_SHORT(pkt->LE_pk_receipt));
			ch_free((char*)pkt);
			if (conn->cn_thead != NOPKT)
				chretran(conn, CHSHORTTIME);
			break;
		default:
			debug(DPKT|DABNOR, printf("bad opcode:%d\n", pkt->pk_op));
			send_los(pkt, "Bad opcode");
			/* should we do clsconn here? */
		}
	/*
	 * Connection is neither waiting for an OPEN nor OPEN.
	 */
	} else {
		debug(DPKT|DABNOR,printf("Packet for conn #%x (not open) state=%d, op:%d\n", CH_INDEX_SHORT(conn->cn_Lidx), conn->cn_state, pkt->pk_op));
		send_los(pkt, "Connection is closed");
	}
	debug(DPKT, printf("rcvpkt: at end"));
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

	debug(DPKT, printf(
	       "reflect: to (%o %o:%d,%d) from (%o %o:%d,%d) pkn %o ackn %o\n",
	       ph->ph_daddr.subnet, ph->ph_daddr.host,
	       ph->ph_didx.tidx, ph->ph_didx.uniq,
	       ph->ph_saddr.subnet, ph->ph_saddr.host,
	       ph->ph_sidx.tidx, ph->ph_sidx.uniq,
	       LE_TO_SHORT(ph->LE_ph_pkn), 
	       LE_TO_SHORT(ph->LE_ph_ackn)));
#endif
	sendctl(pkt);
}

/*
 * Process a received data packet - or an EOF packet which is mostly treated
 * the same.
 */
void
rcvdata(struct connection *conn, struct packet *pkt)
{
	register struct packet *npkt;

	debug(DPKT,(prpkt(pkt,"DATA"),printf("\n")) );
	if (cmp_gt(LE_TO_SHORT(pkt->LE_pk_pkn), conn->cn_rread + conn->cn_rwsize)) {
		debug(DPKT|DABNOR,printf("Discarding data out of window\n"));
		debug(DPKT|DABNOR,printf("pkt->pkn is %x conn->cn_rread %x",
		       LE_TO_SHORT(pkt->LE_pk_pkn), conn->cn_rread));
		debug(DPKT|DABNOR,printf("conn->cn_rwsize is %x sum %x",
		       conn->cn_rread, conn->cn_rread + conn->cn_rwsize));
		ch_free((char*)pkt);
		return;
	}
	receipt(conn, LE_TO_SHORT(pkt->LE_pk_ackn), LE_TO_SHORT(pkt->LE_pk_ackn));
	if (cmp_le(LE_TO_SHORT(pkt->LE_pk_pkn), conn->cn_rlast)) {
		debug(DPKT,printf("Duplicate data packet\n"));
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
	     	LE_TO_SHORT(npkt->LE_pk_pkn) == (unsigned short)(conn->cn_rlast + 1);
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
		debug(DPKT,printf("new ordered data packet\n"));
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
				 cmp_gt(LE_TO_SHORT(pkt->LE_pk_pkn), LE_TO_SHORT(npkt->LE_pk_pkn)); )

			pkt->pk_next = npkt;	/* save the last pkt here */

if (pkt->pk_next == pkt)
{showpkt("rcvdata", pkt);
 panic("rcvdata: pkt->pk_next = pkt");}

		if (npkt != NOPKT && LE_TO_SHORT(pkt->LE_pk_pkn) == LE_TO_SHORT(npkt->LE_pk_pkn)) {
			debug(DPKT|DABNOR,
				printf("Duplicate out of order packet\n"));
			pkt->pk_next = NOPKT;
			makests(conn, pkt);
			reflect(pkt);
		} else {
			if (npkt == conn->cn_routorder)
				conn->cn_routorder = pkt;
			else {
				pkt->pk_next->pk_next = pkt;

if (pkt->pk_next->pk_next == pkt->pk_next)
{ showpkt("rcvdata",pkt->pk_next);
  panic("rcvdata: pkt->pk_next->pk_next = pkt->next");}

			}

			pkt->pk_next = npkt;
if (pkt == npkt) 
{showpkt("rcvdata",pkt);
 panic("rcvdata: pkt= npkt");}

			debug(DPKT|DABNOR,
				printf("New out of order packet\n"));
		}
	}
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
		     pktl != NOPKT && cmp_le(LE_TO_SHORT(pktl->LE_pk_pkn), recnum);
		     pktl = pkt) {

if (pkt->pk_next == pkt) 
{showpkt("receipt",pkt);
panic("receipt: pkt->pk_next = pkt");}

			pkt = pktl->pk_next;
			ch_free((char*)pktl);
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
	if (cmp_gt(acknum, conn->cn_tacked)) {
		if (cmp_gt(acknum, conn->cn_tlast))
			debug(DABNOR, (printf("Invalid acknowledgment(%d,%d)\n",
				acknum, conn->cn_tlast)));
		else {
			conn->cn_tacked = acknum;
			OUTPUT(conn);
		}
	}
}

/*
 * Send a LOS in response to the given packet.
 * Don't bother if the packet is itself a LOS or a CLS since the other
 * end doesn't care anyway and would only return it again.
 * Append the host name to the error message.
 */
void
sendlos(struct packet *pkt, char *str, int len)
{
	debug(DCONN,printf("sendlos() %s\n", str));

	if (pkt->pk_op == LOSOP || pkt->pk_op == CLSOP)
		ch_free((char*)pkt);
	else {
		char *cp;
		int length;
		
		for (cp = Chmyname, length = 0; *cp && length < CHSTATNAME;) {
			length++;
			cp++;
		}
		if ((pkt = pktstr(pkt, str, len + length + sizeof("(from )") - 1))) {
			chmove("(from ", &pkt->pk_cdata[len], 6);
			chmove(Chmyname, &pkt->pk_cdata[len + 6], length);
			chmove(")", &pkt->pk_cdata[len + 6 + length], 1);
			pkt->pk_op = LOSOP;
			reflect(pkt);
		}
	}
}

/*
 * Process a received BRD
 */
void
rcvbrd(struct packet *pkt)
{
	int bitlen = LE_TO_SHORT(pkt->LE_pk_ackn);
	
	SET_CH_ADDR(pkt->pk_daddr, Chmyaddr);
	chmove(&pkt->pk_cdata[bitlen], pkt->pk_cdata, bitlen);
	SET_PH_LEN(pkt->pk_phead, PH_LEN(pkt->pk_phead) - bitlen);
	pkt->LE_pk_ackn = 0;
	rcvrfc(pkt);
}

/*
 * Process a received RFC/BRD
 */
void
rcvrfc(struct packet *pkt)
{
	struct connection *conn, **conptr;
	struct packet **opkt, *pktl;

	debug(DPKT,prpkt(pkt,"RFC/BRD"));
	debug(DPKT,printf("contact = %s\n", pkt->pk_cdata));
	/*
	 * Check if this is a duplicate RFC, and if so throw it away,
	 * and retransmit the OPEN.
	 */
	for (conptr = &Chconntab[0]; conptr < &Chconntab[CHNCONNS];)
		if ((conn = *conptr++) != NOCONN &&
		    CH_INDEX_SHORT(conn->cn_Fidx) == CH_INDEX_SHORT(pkt->pk_sidx) &&
		    CH_ADDR_SHORT(conn->cn_faddr) == CH_ADDR_SHORT(pkt->pk_saddr)) {
			if (conn->cn_state == CSOPEN) {
				debug(DPKT|DABNOR,
					printf("Rcvrfc: Retransmitting open chan #%x\n",
					       CH_INDEX_SHORT(conn->cn_Lidx)));
				if (conn->cn_thead != NOPKT)
					chretran(conn, CHSHORTTIME);
			} else {
				debug(DPKT|DABNOR,
					printf("Rcvrfc: Duplicate RFC: %x\n",
						CH_INDEX_SHORT(conn->cn_Lidx)));
			}
			ch_free((char*)pkt);
			return;
		}
	/*
	 * Scan the listen list for a listener and if one is found
	 * open the connection and remove the listen packet from the
	 * listen list.
	 */
	for (opkt = &Chlsnlist; (pktl = *opkt) != NOPKT; opkt = &pktl->pk_next)
		if(concmp(pkt, pktl->pk_cdata, PH_LEN(pktl->pk_phead))) {
			conn = Chconntab[pktl->pk_stindex];
			*opkt = pktl->pk_next;
			ch_free((char*)pktl);
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
#if 0
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
			if(CH_INDEX_SHORT(pktl->pk_sidx) == CH_INDEX_SHORT(pkt->pk_sidx) &&
			   CH_ADDR_SHORT(pktl->pk_saddr) == CH_ADDR_SHORT(pkt->pk_saddr)) {
				debug(DPKT/*|DABNOR*/,printf("Rcvrfc: Discarding duplicate Rfc on Chrfclist\n"));
				ch_free((char*)pkt);
				return;
			}
		} while ((pktl = pktl->pk_next) != NOPKT);
		Chrfctail->pk_next = pkt;
		Chrfctail = pkt;
	}
	debug(DCONN,printf("Rcvrfc: Queued Rfc on Chrfclist: '%c%c%c%c'\n",
		pkt->pk_cdata[0], pkt->pk_cdata[1], pkt->pk_cdata[2],
		pkt->pk_cdata[3]));
	RFCINPUT;
}
/*
 * An RFC has matched a listener, either by an RFC coming and finding a match
 * on the listener list, or by a listen being done and matching an RFC on the
 * unmatched RFC list. So we change the state of the connection to CSRFCRCVD
 */
void
lsnmatch(struct packet *rfcpkt, struct connection *conn)
{
	debug(DCONN,printf("Conn #%x: LISTEN matched \n", CH_INDEX_SHORT(conn->cn_Lidx)));
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
	  SET_CH_ADDR(conn->cn_laddr, Chmyaddr);
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
			ch_free((char*)pkt);
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
	debug(DPKT,printf("Rcvrfc: Comparing %s and %s\n", rfcstr, lsnstr));
	debug(DPKT,printf("rfcpkt->pk_len = %d lsnlen = %d", 
	       PH_LEN(rfcpkt->pk_phead),
	       lsnlen));

	for (rfclen = PH_LEN(rfcpkt->pk_phead); rfclen; rfclen--, lsnlen--) {
	  if (lsnlen <= 0) {
	    debug(DPKT,printf("reached lsnlen = %d *rfcstr is '%c' %x", lsnlen,
		   *rfcstr, *rfcstr));
	    debug(DPKT,printf("returning 1"));
	    return 1;
	    /* return ((*rfcstr == ' ') ? 1 : 0); */
	  }
	  else if (*rfcstr++ != *lsnstr++) {
	    debug(DPKT,printf("failed to match character %x and %x",
			      *(rfcstr-1), *(lsnstr-1)));
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

	debug(DPKT, ( prpkt(pkt,"RUT"),printf("\n") ) );
	rd = pkt->pk_rutdata;
	if (pkt->pk_ssubnet >= CHNSUBNET)
		printf("CHAOS: bad subnet %d in RUT pkt\n", pkt->pk_ssubnet);
	else if (Chroutetab[pkt->pk_ssubnet].rt_type != CHDIRECT)
		printf("CHAOS: RUT pkt from unconnected subnet %d\n",
			pkt->pk_ssubnet);
	else for (i = PH_LEN(pkt->pk_phead); i; i -= sizeof(struct rut_data), rd++) {
		if (LE_TO_SHORT(rd->LE_pk_subnet) >= CHNSUBNET) {
			debug(DABNOR, printf("CHAOS: bad subnet %d in RUT pkt\n",
				LE_TO_SHORT(rd->LE_pk_subnet)));
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
			printf("CHAOS: Illegal chaos routing table entry %d",
				r->rt_type);
		}
	}
	ch_free((char*)pkt);
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
	
	ch_free((char*)pkt);
	for (i = 0, r = Chroutetab; r < &Chroutetab[CHNSUBNET]; r++)
		if (r->rt_type == CHDIRECT)
			i++;
	i *= sizeof(struct stathead) + sizeof(struct statxcvr);
	i += CHSTATNAME;
	if ((pkt = pkalloc(i, 1)) == NOPKT)
		return;
	SET_CH_ADDR(pkt->pk_daddr, saddr);
	SET_CH_INDEX(pkt->pk_didx, sidx);
	pkt->pk_type = 0;
	pkt->pk_op = ANSOP;
	pkt->pk_next = NOPKT;
	SET_CH_ADDR(pkt->pk_saddr, daddr);
	SET_CH_INDEX(pkt->pk_sidx, 0);
	pkt->LE_pk_pkn = pkt->LE_pk_ackn = 0;
	SET_PH_LEN(pkt->pk_phead, i);

	chmove(Chmyname, pkt->pk_status.sb_name, CHSTATNAME);
	sp = &pkt->pk_status.sb_data[0];
	for (r = Chroutetab; r < &Chroutetab[CHNSUBNET]; r++)
		if (r->rt_type == CHDIRECT) {
			xp = r->rt_xcvr;
			sp->LE_sb_ident = LE_TO_SHORT(0400 + xp->xc_subnet);
			sp->LE_sb_nshorts = LE_TO_SHORT(sizeof(struct statxcvr) / sizeof(short));
			sp->sb_xstat = xp->xc_xstat;
#ifdef pdp11
			swaplong(&sp->sb_xstat,
				sizeof(struct statxcvr) / sizeof(long));
#endif
			sp = (struct statdata *)((char *)sp +
				sizeof(struct stathead) +
				sizeof(struct statxcvr));
		}
	sendctl(pkt);
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
	

	ch_free((char*)pkt);
	if ((pkt = pkalloc(CHNSUBNET * 4, 1)) != NOPKT) {
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
		SET_CH_ADDR(pkt->pk_daddr, saddr);
		SET_CH_INDEX(pkt->pk_didx, sidx);
		pkt->pk_type = 0;
		pkt->pk_op = ANSOP;
		pkt->pk_next = NOPKT;
		SET_CH_ADDR(pkt->pk_saddr, daddr);
		SET_CH_INDEX(pkt->pk_sidx, 0);
		pkt->LE_pk_pkn = pkt->LE_pk_ackn = 0;
		SET_PH_LEN(pkt->pk_phead, CHNSUBNET * 4);
		sendctl(pkt);
	}
}
/*
 * process a RFC for contact name TIME 
 */
void
timerfc(struct packet *pkt)
{
	long t;

	pkt->pk_op = ANSOP;
	pkt->pk_next = NOPKT;
	pkt->LE_pk_pkn = pkt->LE_pk_ackn = 0;
	SET_PH_LEN(pkt->pk_phead, sizeof(long));
	ch_time(&t);
	pkt->pk_ldata[0] = LE_TO_LONG(t);
#ifdef pdp11
	swaplong(&pkt->pk_ldata, 1);
#endif
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
#ifdef pdp11
	swaplong(&pkt->pk_ldata, 1);
#endif
	reflect(pkt);
}


#ifdef DEBUG_CHAOS
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
	printf("op=%s(%s) num=%d len=%d fc=%d; dhost=%o didx=%x; shost=%o sidx=%x\npkn=%d ackn=%d",
		str, pkt->pk_op == 128 ? "DAT" : pkt->pk_op == 129 ? "SYN" : pkt->pk_op == 192 ? "DWD" : opcodetable[pkt->pk_op],
		LE_TO_SHORT(pkt->LE_pk_pkn), PH_LEN(pkt->pk_phead), 
	        PH_FC(pkt->pk_phead), pkt->pk_dhost,
		CH_INDEX_SHORT(pkt->pk_phead.ph_didx), pkt->pk_shost, 
	        CH_INDEX_SHORT(pkt->pk_phead.ph_sidx),
	       (unsigned)LE_TO_SHORT(pkt->LE_pk_pkn), 
	       (unsigned)LE_TO_SHORT(pkt->LE_pk_ackn));
}
#endif

void
showpkt(char *str, register struct packet *pkt)
{
  printf("%s: pkt %X, pkt->next %X\n", str,pkt, pkt->pk_next);
  prpkt(pkt,"");
}
