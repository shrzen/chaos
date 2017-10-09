#include "../chunix/chconf.h"
#include "../chunix/chsys.h"
#include <chaos/chaos.h>
#include <chaos/dev.h>
#include "../h/param.h"
#include "../h/inode.h"
#include "../h/file.h"
#include "../h/tty.h"
#include "../h/dir.h"
#include "../h/user.h"
#include "../h/conf.h"
/*
 * This file contains functions for treating a chaos channel as a UNIX tty
 */

/*
 * Do the additional work necessary to open a channel as a UNIX tty.
 */
chttyopen(conn, dev)
register struct connection *conn;
{
	struct tty *tp;
	int chttystart(), chttyinput();

	/*
	 * Allocate a tty structure - it would be
	 * nicer if we made sure we didn't over-
	 * allocate...
	 */
	tp = conn->cn_ttyp = (struct tty *)ch_alloc(sizeof(struct tty), 0);
	clear((char *)tp, sizeof(struct tty));
	tp->t_addr = (caddr_t)conn;
	tp->t_oproc = chttystart;
	tp->t_iproc = chttyinput;
	tp->t_state = ISOPEN | CARR_ON;
	ttychars(tp);
	tp->t_flags = ODDP|EVENP|ECHO;
	tp->t_ospeed = tp->t_ispeed = B9600;
	conn->cn_mode = CHTTY;
	(*linesw[tp->t_line].l_open)(dev, tp);
}
/*
 * Close the tty associated with the given connection.
 */
chttyclose(conn)
register struct connection *conn;
{
	register struct tty *tp;

	if (conn->cn_mode != CHTTY || (tp = conn->cn_ttyp) == (struct tty *)0)
		panic("no tty for chaos channel");
	(*linesw[tp->t_line].l_close)(tp);
	/* HUPCLS bit is superfluos - can't NOT hangup! */
	ttyclose(tp);
	LOCK;
	conn->cn_mode = CHSTREAM;
	ch_free((caddr_t)conn->cn_ttyp);
	conn->cn_ttyp = NULL;
	UNLOCK;
}

/*
 * Read from a chaos channel that is a tty.
 */
chttyread(conn)
register struct connection *conn;
{
	/*
	 * Since ttys are quite possibly interactive, be sure
	 * to flush any output when input is desired.  It would
	 * be soon anyway due to timeouts.
	 */
	spl6();
	if (conn->cn_toutput != NOPKT && !chtfull(conn))
		ch_sflush(conn);
	spl0();
	(*linesw[conn->cn_ttyp->t_line].l_read)(conn->cn_ttyp);
}
/*
 * Write to a chaos channel that is a tty.
 */
chttywrite(conn)
register struct connection *conn;
{
	(*linesw[conn->cn_ttyp->t_line].l_write)(conn->cn_ttyp);
}
/*
 * Perform ioctl for the chaos tty.
 */
chttyioctl(conn, cmd, addr, flag)
register struct connection *conn;
caddr_t addr;
{
	register struct tty *tp;

	tp = conn->cn_ttyp;
	cmd = (*linesw[tp->t_line].l_ioctl)(tp, cmd, addr);
	if (cmd == 0)
		return;
	if (ttioctl(tp, cmd, addr, flag)) {
		if (cmd==TIOCSETP || cmd==TIOCSETN) {
			/*
			 * Send virtual terminal codes here?
			 */
/*			chparam(conn); */
		}
		return;
	} else switch(cmd) {
	/*
	 * We need a remote terminal protocol here. - Yick.
	 */
	case TIOCSBRK:
		/* maybe someday a code for break on output ? */
		break;
	case TIOCCBRK:
		break;
	case TIOCSDTR:
		break;
	case TIOCCDTR:
		break;
	default:
		u.u_error = ENOTTY;
	u.u_error = ENXIO;
}
	}
/*
 * Interrupt routine called when a new packet is available for a tty channel
 * Basically empty the packet into the tty system.
 * Called both from interrupt level when the read packet queue becomes
 * non-empty and also (at high priority) from top level.  THe top level
 * call is needed to retrieve data not queued at interrupt time due to
 * input queue high water mark reached.
 */
chttyrint(conn)
struct connection *conn;
{
	register struct packet *pkt;
	register struct tty *tp = conn->cn_ttyp;
	register char *cp;

	while ((pkt = conn->cn_rhead) != NOPKT) {
		if (ISDATOP(pkt)) {
			for (cp = &pkt->pk_cdata[conn->cn_roffset];
			     pkt->pk_len != 0; pkt->pk_len--)
				if (tp->t_rawq.c_cc + tp->t_canq.c_cc >= TTYHOG) {
					conn->cn_roffset = cp - pkt->pk_cdata;
					return;
				} else
					(*linesw[tp->t_line].l_rint)(*cp++ & 0377, tp);
		}
		ch_read(conn);
	}
	if (conn->cn_toutput != NOPKT)
		ch_sflush(conn);
}
/*
 * Get more data if possible.
 */
chttyinput(tp)
register struct tty *tp;
{
	register int opl = spl6();

	chttyrint((struct connection *)tp->t_addr);
	splx(opl);
}
/*
 * Interrupt routine called when more packets can again be sent after things
 * block due to window full.
 */
chttyxint(conn)
register struct connection *conn;
{
	register struct tty *tp = conn->cn_ttyp;
	register int s = spl6();

	tp->t_state &= ~BUSY;
	if (tp->t_line)
		(*linesw[tp->t_line].l_start)(tp);
	else
		chttystart(tp);
	splx(s);
}
/*
 * Restart transmission
 */
chttystart(tp)
register struct tty *tp;
{
	register struct connection *conn = (struct connection *)tp->t_addr;
	register struct packet *pkt;
	int sps, cc;
	extern int ttrstrt();

	sps = spl6();
	if (conn->cn_state == CSOPEN &&
	    (tp->t_state & (TIMEOUT|BUSY|TTSTOP)) == 0) {
		while (tp->t_outq.c_cc != 0) {
			if ((pkt = conn->cn_toutput) == NOPKT) {
				cc = chtfull(conn) ? CHMAXDATA :
					chroundup(tp->t_outq.c_cc * 2);
				if ((pkt = pkalloc(cc, 1)) == NOPKT) {
					cc = CHMINDATA;
					if ((pkt = pkalloc(cc, 1)) == NOPKT) {
						timeout(ttrstrt, (caddr_t)tp,
							CHBUFTIME);
						tp->t_state |= TIMEOUT;
						splx(sps);
						return;
					}
				}
				pkt->pk_op = DATOP;
				pkt->pk_type = 0;
				pkt->pk_lenword = 0;
				conn->cn_troom = cc;
				conn->cn_toutput = pkt;
			}
			if ((cc = tp->t_outq.c_cc) > conn->cn_troom)
				cc = conn->cn_troom;
			if (cc != 0) {
				(void)q_to_b(&tp->t_outq,
					&pkt->pk_cdata[pkt->pk_len], cc);
				pkt->pk_time = Chclock;
				pkt->pk_lenword += cc;
			}
			if (tp->t_state & ASLEEP &&
			    tp->t_outq.c_cc <= TTLOWAT(tp)) {
				tp->t_state &= ~ASLEEP;
				wakeup((caddr_t)&tp->t_outq);
			}
			if ((conn->cn_troom -= cc) == 0)
				if (chtfull(conn)) {
					tp->t_state |= BUSY;
					splx(sps);
					return;
				} else
					ch_sflush(conn);
		}
		/*
		 * Here we must deal with a partially full packet.
		 * For now, just get rid of it.
		 * We might schedule a timeout flush or something.
		 * Or maybe check if we are at interrupt level...
		if (conn->cn_toutput)
			if (chtfull(conn))
				tp->t_state |= BUSY;
			else
				ch_sflush(conn);
		*/
	}
	splx(sps);
}
/*
 * Process a connection state change for a tty.
 */
chttynstate(conn)
register struct connection *conn;
{
	register struct tty *tp;

	tp = conn->cn_ttyp;
	switch (conn->cn_state) {
	case CSOPEN:
		if ((tp->t_state & CARR_ON) == 0) {
			wakeup((caddr_t)&tp->t_rawq);
			tp->t_state |= CARR_ON;
		}
		break;
	case CSCLOSED:
	case CSINCT:
	case CSLOST:
		if ((tp->t_state & CARR_ON) &&
		    (tp->t_local & LNOHANG) == 0) {
			if (tp->t_state & ISOPEN) {
				gsignal(tp->t_pgrp, SIGHUP);
#ifdef SIGCONT
				gsignal(tp->t_pgrp, SIGCONT);
#endif
			}
			tp->t_state &= ~CARR_ON;
		}
		break;
	default:
		panic("chttystate");
	}
}


