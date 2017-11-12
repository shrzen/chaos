/*
 *	$Source: /home/ams/c-rcs/chaos-2000-07-03/kernel/chncp/chch11.c,v $
 *	$Author: brad $
 *	$Locker:  $
 *	$Log: chch11.c,v $
 *	Revision 1.1.1.1  1998/09/07 18:56:07  brad
 *	initial checkin of initial release
 *
 * Revision 1.1  84/06/12  20:27:07  jis
 * Initial revision
 * 
 */
#ifndef lint
static char *rcsid_chch11_c = "$Header: /home/ams/c-rcs/chaos-2000-07-03/kernel/chncp/chch11.c,v 1.1.1.1 1998/09/07 18:56:07 brad Exp $";
#endif lint
#include "../h/chaos.h"
#include "../chunix/chsys.h"
#include "../chunix/chconf.h"
#include "../chncp/chncp.h"
/*
 * Chaos device driver for the ch11 (CHAOS-11) broadcast network
 * interface.
 */
struct chxcvr ch11xcvr[NCHCH];
#define xc_chrtries	xc_info.xc_ch11.ch_rtries
int chchstart(), chchreset();

#ifdef vax
#include "../h/buf.h"
#include "../vaxuba/ubavar.h"
int chchndev;
/*
 * Autoconfiguration support for VMUNIX
 */
int	chchprobe(), chchattach(), chchintr();
struct	uba_device *chchdinfo[NCHCH];
unsigned short	chchstd[] = { 0164140, 0165140, 0166140 };
struct	uba_driver chchdriver =
	{ chchprobe, 0, chchattach, 0, chchstd, "chch", chchdinfo };

chchprobe(reg)
	caddr_t reg;
{
	register int br, cvec;

#ifdef lint
	br = 0; cvec = br; br = cvec;
#endif
	((struct ch11 *)reg)->ch_csr = CHTCLR;
	((struct ch11 *)reg)->ch_csr = CHTIEN;
	DELAY(10000);
	((struct ch11 *)reg)->ch_csr = 0;
	return (sizeof(struct ch11));
}

/*ARGSUSED*/
chchattach(ui)
	register struct uba_device *ui;
{
	
	if (++chchndev > NCHCH)
		panic("too many ch11's");
	chchsetup(&ch11xcvr[chchndev - 1], (struct ch11 *)ui->ui_addr);
}
#else vax
int chchndev = NCHCH;
int chchaddr[NCHCH] = { CHCH_ADDR };
#endif vax
/*
 * Do the run-time initialization of the given transceiver.
 */
chchsetup(xp, cp)
register struct chxcvr *xp;
register struct ch11 *cp;
{
	xp->xc_devaddr = (unsigned *)cp;
	xp->xc_addr = cp->ch_myaddr;
	xp->xc_start = chchstart;
	xp->xc_reset = chchreset;
	xp->xc_xmit = chxmitpkt;
	xp->xc_cost = CHCCOST;
	printf("CHAOS cable: subnet: %o, host: %o, (%o)\n",
		xp->xc_subnet, xp->xc_host, xp->xc_addr);
	if (xp->xc_subnet >= CHNSUBNET)
		panic("bad ch11 address");
}

/*
 * chaos11 initialization. Also does initialization to the static xcvr
 * structure since initialized unions don't work anyway.
 */
chchinit()
{
	register struct chxcvr *xp;
	register int i;

	for (i = 0, xp = &ch11xcvr[0]; xp < &ch11xcvr[chchndev]; xp++, i++) {
#ifndef vax
		chchsetup(xp, (struct ch11 *)chchaddr[i]);
#endif not vax
		chchreset(xp);
	}
}
/*
 * Reset things from an unknown state
 */
chchreset(xp)
register struct chxcvr *xp;
{
	register struct ch11 *cp = (struct ch11 *)xp->xc_devaddr;

	if (cp == 0)
		return;
	chattach(xp);
	cp->ch_csr = CHRST | CHRIEN | CHREN | CHTIEN;
}

/*
 * Start output on an idle line
 */
chchstart(xp)
register struct chxcvr *xp;
{
	if (xp->xc_tpkt != NOPKT)
		panic("chchstart: already busy");
	chchtint(xp);
}

/*
 * Receiver interrupt
 */
chchrint(xcvr)
struct chxcvr *xcvr;
{
	register nshorts;
	register struct ch11 *cp;
	register short *p;
	register struct chxcvr *xp = xcvr;	/* needed so xp is last reg */

	cp = (struct ch11 *)xp->xc_devaddr;
	nshorts = (((cp->ch_rbc & 07777) + 1) >> 4) - 3;
	if ((cp->ch_rbc & 017) != 017 || nshorts < 8)
		xp->xc_leng++;		/* bit count not multiple of 16 */
	else if (cp->ch_csr & CHCRC)
		xp->xc_crcr++;		/* crc error during reception */
	else if ((xp->xc_rpkt = hpkalloc(nshorts)) == NOPKT)
		xp->xc_rej++;		/* can't allocate a packet */
	else {
		p = (short *)&xp->xc_rpkt->pk_phead;
		do {
			*p++ = cp->ch_rbf;
		} while (--nshorts);
		nshorts = cp->ch_rbf;
		nshorts = cp->ch_rbf;
		nshorts = cp->ch_rbf;
		if (cp->ch_csr & CHCRC) {
			xp->xc_crci++;	/* crc error during readout */
			ch_free((char *)xp->xc_rpkt);
		} else
			rcvpkt(xp);
	}
	xp->xc_lost += (cp->ch_csr >> CHLCPOS) & CHLCMASK;
	cp->ch_csr |= CHREN | CHRIEN;	/* CHRIEN is "not really needed" */
}
/*
 * Transmit interrupt
 */
chchtint(xcvr)
struct chxcvr *xcvr;
{
	register struct ch11 *cp;
	register struct packet *pkt;	/* shouldn't be reg on pdp11 */

{
	register struct chxcvr *xp = xcvr;

	LOCK;
	cp = (struct ch11 *)xp->xc_devaddr;
	cp->ch_csr &= ~CHTIEN;
	if ((pkt = xp->xc_tpkt) != NOPKT) {	/* if a real interrupt */
		if (cp->ch_csr & CHABRT) {	/* should retry here first */
			xp->xc_abrt++;
			if (++xp->xc_chrtries < 5)
				goto retry;
		} else
			xp->xc_xmtd++;
		xmitdone(pkt);
		xp->xc_chrtries = 0;
	}
	if ((pkt = xmitnext(xp)) == NOPKT)
		return;
retry:	;
}{
	register int nshorts;
	register short *p;

	nshorts = (sizeof(struct pkt_header) + pkt->pk_len + sizeof(short)-1)
			/* / sizeof(short) */ >> 1;
	debug(DTRANS,printf("Trans size=%d, %x\n", nshorts, pkt));
	cp->ch_csr |= CHTCLR;
	p = (short *)&pkt->pk_phead;
	do {
		cp->ch_wbf = *p++;
	} while (--nshorts);
	cp->ch_wbf = pkt->pk_xdest;
	nshorts = cp->ch_xmt;	/* start transmission */
	cp->ch_csr |= CHTIEN;
}
}
/*
 * CHAOS-11 interrupt
 * call chchrint and/or chchtint
 */
chchintr(dev)
{
	register struct chxcvr *xp = &ch11xcvr[dev];
	register struct ch11 *cp = (struct ch11 *)xp->xc_devaddr;

	if (cp->ch_csr & (CHTDN | CHRDN)) {
		if (cp->ch_csr & CHRDN)
			if (cp->ch_csr & CHRIEN)
				chchrint(xp);
		if (cp->ch_csr & CHTDN)
			if (cp->ch_csr & CHTIEN)
				chchtint(xp);
	} else
		debug(DTRANS, printf("spurious CHAOS11 interrupt\n"));
}
