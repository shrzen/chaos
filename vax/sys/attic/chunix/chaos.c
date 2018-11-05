/*
 *	$Source: /afs/dev.mit.edu/source/src77/bsd-4.3/vax/sys/attic/chunix/RCS/chaos.c,v $
 *	$Author: cfields $
 *	$Locker:  $
 *	$Log: chaos.c,v $
 * Revision 1.1  1994/09/16  11:06:35  cfields
 * Initial revision
 *
 * Revision 1.6  87/04/06  10:28:05  rc
 * This is essentially the same as revision 1.5 except that the RCS log
 * was not updated to reflect the changes. In particular, calls to spl6()
 * are replaced with calls to splimp() in order to maintain consistency
 * with the rest of the Chaos code.
 * 
 * Revision 1.5  87/04/06  07:52:49  rc
 * Modified this file to include the select stuff for the ChaosNet. 
 * Essentially, the new distribution from "think" was integrated
 * together with the local modifications at MIT to reflect this
 * new version.
 * 
 * Revision 1.4.1.1  87/03/30  21:06:16  root
 * The include files are updated to reflect the correct "include" directory.
 * These changes were not previously RCSed. The original changes did not
 * have lines of the sort ... #include "../netchaos/...."
 * 
 * Revision 1.4  86/10/07  16:16:32  mbm
 * CHIOCGTTY ioctl; move DTYPE_CHAOS to file.h
 * 
 * Revision 1.2  84/11/05  18:24:31  jis
 * Add checks for allocation failure after some calls to pkalloc
 * so that when pkalloc fails to allocate a packet, the system
 * no longer crashes.
 * 
 * Revision 1.1  84/11/05  17:35:32  jis
 * Initial revision
 * 
 */

#ifndef lint
static char *rcsid_chaos_c = "$Header: /afs/dev.mit.edu/source/src77/bsd-4.3/vax/sys/attic/chunix/RCS/chaos.c,v 1.1 1994/09/16 11:06:35 cfields Exp $";
#endif	lint

/*
 * UNIX device driver interface to the Chaos N.C.P.
 */
#include "../h/chaos.h"
#include "../chunix/chsys.h"
#include "../chunix/chconf.h"
#include "../chncp/chncp.h"
#ifndef BSD4_3
#include "../h/vnode.h"
#endif
#include "../h/file.h"
#include "../h/dir.h"
#include "../h/user.h"
#include "../h/conf.h"
#include "../h/buf.h"
#include "../h/systm.h"
#include "../h/uio.h"
#include "../h/kernel.h"
#include "../h/ioctl.h"
#include "../h/proc.h"

#ifdef vax
#ifndef BSD4_3
#include "cht.h"
#endif
#endif

static int		initted;	/* NCP initialization flag */
int			Rfcwaiting;	/* Someone waiting on unmatched RFC */

#define CHIOPRIO	(PZERO+1)	/* Just interruptible */
#ifdef CHAOSDEBUG
#define ASSERT(x,y)	if(!(x)) printf("%s: Assertion failure\n",y);
#else
#define ASSERT(x,y)
#endif
#ifndef vax
#define setjmp save
#endif
#ifdef BSD42
/*
 * 4.2 BSD glue between generic file descriptor and chaos connection code.
 */
int chf_rw(), chf_ioctl(), chf_select(), chf_close(), chread(), chwrite();
struct fileops chfileops = { chf_rw, chf_ioctl, chf_select, chf_close };

chf_rw(fp, rw, uio)
	register struct file *fp;
	enum uio_rw rw;
	struct uio *uio;
{
	int ret;

	ASSERT(fp->f_ops == &chfileops, "chf_rw ent")
	ret = (rw == UIO_READ ? chread : chwrite)
		((struct connection *)fp->f_data, (int)uio);
	ASSERT(fp->f_ops == &chfileops, "chf_rw exit")
	return ret;
}	
chf_ioctl(fp, cmd, value)
	register struct file *fp;
	int cmd;
	caddr_t value;
{
	int ret;
	ASSERT(fp->f_ops == &chfileops, "chf_ioctl ent")
	ret = chioctl((struct connection *)fp->f_data, cmd, value);
	ASSERT(fp->f_ops == &chfileops, "chf_ioctl exit")
	return ret;
}

/* select support -- Bruce Nemnich, 6 May 85 */
chf_select(fp, which)
	struct file *fp;
	int which;
{
  	register struct connection *conn = (struct connection *)fp->f_data;
	register int s = splimp();
	register struct proc *p;
	int retval = 0;
	ASSERT(fp->f_ops == &chfileops, "chf_select ent")

	switch (which) {

	case FREAD:
		if (chrempty(conn)) {
		  if ((p = conn->cn_rsel) && p->p_wchan == (caddr_t)&selwait)
		    conn->cn_flags |= CHSELRCOLL;
		  else conn->cn_rsel = u.u_procp;
		}
		else retval = 1;
		break;

	case FWRITE:
		if (chtfull(conn)) {
		  if ((p = conn->cn_tsel) && p->p_wchan == (caddr_t)&selwait)
		    conn->cn_flags |= CHSELTCOLL;
		  else conn->cn_tsel = u.u_procp;
		}
		else retval = 1;
		break;
	}
	splx(s);
	ASSERT(fp->f_ops == &chfileops, "chf_select exit")
	return retval;
}


chf_close(fp)
	register struct file *fp;
{
	register struct connection *conn = (struct connection *)fp->f_data;

	/*
	 * If this connection has been turned into a tty, then the
	 * tty owns it and we don't do anything.
	 */
	
	ASSERT(fp->f_ops == &chfileops, "chf_close ent")
	if (conn && conn->cn_mode != CHTTY)
		 chclose(conn);
}
#define UCOUNT ((struct uio *)uio)->uio_resid
#define UREAD(from, count) uiomove(from, count, UIO_READ, uio)
#define UWRITE(to, count) uiomove(to, count, UIO_WRITE, uio)
/* !!!!!!!! This should really be defined in ../h/file.h !!!!!!!! */
/* #define DTYPE_CHAOS 8 */
/* .... so I put it there.   -mbm */

#else
#define CHFCONN(fp)	((struct connection *)(fp)->f_un.f_chan)
#define UREAD(from, count) (iomove(from, count, B_READ), u.u_error)
#define UWRITE(to, count) (iomove(to, count, B_WRITE), u.u_error)
#define UCOUNT u.u_count
#endif

/* ARGSUSED */
chwcopy(from, to, count, uio, errorp)
	char *from, *to;
	unsigned count;
	int *errorp;
{
	*errorp = UWRITE(to, count);
	return 0;
}
/* ARGSUSED */
chrcopy(from, to, count, uio, errorp)
	char *from, *to;
	unsigned count;
	int *errorp;
{
	*errorp = UREAD(from, count);
	return 0;
}
/*
 * Access the Chaos NCP.
 * For one of two reasons:
 *	1. To perform basic NCP control functions, via minor device CHURFCMIN
 *	2. To open a connection via CHAOSMIN
 * The actual opening of the connection happens later via the CHIOCOPEN ioctl.
 */
/* ARGSUSED */
chropen(dev, flag)
dev_t dev;
{
	int errno = 0;

	ch_bufalloc();
	/* initialize the NCP somewhere else? */
	if (!initted) {
		chreset();	/* Reset drivers */
		chtimeout();	/* Start clock "process" */
		initted++;
	}
	if (minor(dev) == CHURFCMIN)
		if(Chrfcrcv == 0)
			Chrfcrcv++;
		else
			errno = ENXIO;
#ifdef BSD42
	return errno;
#else
	else	/* Yick. (see mx2.c) */
		CHFCONN(u.u_ofile[u.u_r.r_val1]) = NOCONN;
	u.u_error = errno;
#endif
}

/*
 * Return a connection or return NULL and set *errnop to any error.
 */
struct connection *
chopen(c, wflag, errnop)
register struct chopen *c;
int *errnop;
{
	register struct connection *conn;
	register struct packet *pkt;
	int rwsize, length;

	*errnop = 0;
	length = c->co_clength + c->co_length +
		 (c->co_length ? 1 : 0);
	if (length > CHMAXPKT ||
	    c->co_clength <= 0) {
		*errnop = E2BIG;
		return NOCONN;
	}
	pkt = pkalloc(length, 0);
	if (pkt == NOPKT) {
		*errnop = ENOBUFS;
		return NOCONN;
	}
	if (c->co_length)
	pkt->pk_cdata[c->co_clength] = ' ';
	if (copyin(c->co_contact, pkt->pk_cdata, c->co_clength) ||
	    c->co_length &&
	    copyin(c->co_data, &pkt->pk_cdata[c->co_clength + 1], c->co_length)) {
		ch_free((caddr_t)pkt);
		*errnop = EFAULT;
		return NOCONN;
	}
	rwsize = c->co_rwsize ? c->co_rwsize : CHDRWSIZE;
	pkt->pk_lenword = length;
	conn = c->co_host ? ch_open(c->co_host, rwsize, pkt) : ch_listen(pkt, rwsize);
	if (conn == NOCONN) {
		*errnop = ENXIO;
		return NOCONN;
	}
	if (!c->co_async) {
		/*
		 * We should hang until the connection changes from
		 * its initial state.
		 * If interrupted, flush the connection.
		 */
#ifdef BSD42
		if (setjmp(&u.u_qsave) == 0) {
#else
		if (setjmp(u.u_qsav) == 0) {
#endif
			LOCK;
			while (conn->cn_state == (c->co_host ? CSRFCSENT : CSLISTEN))
				sleep((caddr_t)conn, CHIOPRIO);
			UNLOCK;
			/*
			 * If the connection is not open, the open failed.
			 * Unless is got an ANS back.
			 */
			if (conn->cn_state != CSOPEN &&
			    (conn->cn_state != CSCLOSED ||
			     (pkt = conn->cn_rhead) == NOPKT ||
			     pkt->pk_op != ANSOP)) {
				rlsconn(conn);
				*errnop = EIO;
				return NOCONN;
			}
		}
	}
	if (wflag)
		conn->cn_sflags |= CHFWRITE;
	conn->cn_sflags |= CHRAW;
	conn->cn_mode = CHSTREAM;
	return conn;
}

/* ARGSUSED */
chrclose(dev, flag, conn)
dev_t dev;
register struct connection *conn;
{
	if (minor(dev) == CHURFCMIN) {
		Chrfcrcv = 0;
		freelist(Chrfclist);
		Chrfclist = NOPKT;
		return;
#ifdef BSD42
	}
#else
	} else {
		/*
		 * If this connection has been turned into a tty, then the
		 * tty owns it and we don't do anything.
		 */
		if (conn && conn->cn_mode != CHTTY)
			 chclose(conn);
	}
#endif
}

chclose(conn)
register struct connection *conn;
{
	register struct packet *pkt;

	switch (conn->cn_mode) {
	case CHTTY:
		panic("chclose on tty");
	case CHSTREAM:
		splimp();
#ifdef BSD42
		if (setjmp(&u.u_qsave)) {
#else
		if (setjmp(u.u_qsav)) {
#endif
			pkt = pktstr(NOPKT, "User interrupted", 16);
			if (pkt != NOPKT)
				pkt->pk_op = CLSOP;
			ch_close(conn, pkt, 0);
			goto shut;
		}
		if (conn->cn_sflags & CHFWRITE) {
			/*
			 * If any input packets other than the RFC are around
			 * something is wrong and we just abort the connection
			 */
			while ((pkt = conn->cn_rhead) != NOPKT) {
				ch_read(conn);
				if (pkt->pk_op != RFCOP)
					goto recclose;
			}
			/*
			 * We set this flag telling the interrupt time
			 * receiver to abort the connection if any new packets
			 * arrive.
			 */
			conn->cn_sflags |= CHCLOSING;
			/*
			 * Closing a stream transmitter involves flushing
			 * the last packet, sending an EOF and waiting for
			 * it to be acknowledged.  If the connection was
			 * bidirectional, the reader should have already
			 * read until EOF if everything is to be closed
			 * cleanly.
			 */
		checkfull:
			while (chtfull(conn)) {
				conn->cn_sflags |= CHOWAIT;
				sleep((caddr_t)&conn->cn_thead, CHIOPRIO);
			}
			if (conn->cn_state == CSOPEN ||
			    conn->cn_state == CSRFCRCVD) {
				if (conn->cn_toutput) {
					ch_sflush(conn);
					goto checkfull;
				}
				if (conn->cn_state == CSOPEN)
					(void)ch_eof(conn);
			}
			while (!chtempty(conn)) {
				conn->cn_sflags |= CHOWAIT;
				sleep((caddr_t)&conn->cn_thead, CHIOPRIO);
			}
		} else if (conn->cn_state == CSOPEN) {
			/*
			 * If we are only reading then we should read the EOF
			 * before closing and wiat for the other end to close.
			 */
			if (conn->cn_flags & CHEOFSEEN)
				while (conn->cn_state == CSOPEN)
					sleep((caddr_t)conn, CHIOPRIO);
		}
	recclose:
		spl0();
		/* Fall into... */
	case CHRECORD:	/* Record oriented close is just sending a CLOSE */
		if (conn->cn_state == CSOPEN) {
			pkt = pkalloc(0, 0);
			if (pkt != NOPKT) {
				pkt->pk_op = CLSOP;
				pkt->pk_len = 0;
			}
			ch_close(conn, pkt, 0);
		}
	}
shut:
	spl0();
	ch_close(conn, NOPKT, 1);
	ch_buffree();
}
/*
 * Raw read routine.
 */
chrread(dev, uio)
dev_t dev;
{
	register struct packet *pkt;
	register int errno = 0;

	if(minor(dev) == CHURFCMIN) {
		splimp();
		while ((pkt = ch_rnext()) == NOPKT) {
			Rfcwaiting++;
			sleep((caddr_t)&Chrfclist, CHIOPRIO);
		}
		spl0();	
		if (UCOUNT < pkt->pk_len)
			errno = EIO;
		else
			UREAD((caddr_t)pkt->pk_cdata, pkt->pk_len);
	} else
#ifdef BSD42
		errno = ENXIO;
	return errno;
#else
		{
		register struct connection *conn;
	
		if ((conn = CHFCONN(getf(u.u_arg[0]))) == NOCONN)
			errno = ENXIO;
		else
			errno = chread(conn, 0);
		}
	u.u_error = errno;
#endif
}

/*
 * Return an errno on error
 */
chread(conn, uio)
register struct connection *conn;
{
	register struct packet *pkt;
	register int count;
	
	switch (conn->cn_mode) {
	case CHTTY:
		return ENXIO;
	case CHSTREAM:
		if (conn->cn_state == CSRFCRCVD)
			ch_accept(conn);
		for (count = UCOUNT; UCOUNT != 0; ) {
			int error, n;

			n = ch_sread(conn, (char*)0, (unsigned)UCOUNT, uio, &error);
			if (error)
				return error;
			switch (n) {
			case 0:	/* No data to read */
				if (count != UCOUNT)
					return 0;
				splimp();
				while (chrempty(conn)) {
					conn->cn_sflags |= CHIWAIT;
					sleep((char *)&conn->cn_rhead,
						CHIOPRIO);
				}
				spl0();
				continue;
			case CHEOF:
				return 0;
			default:
				if (n < 0)
					return EIO;
			}
		}
		return 0;
	/*
	 * Record oriented mode gives a one byte packet opcode
	 * followed by the data in the packet.  The buffer must
	 * be large enough to fit the data and the opcode, otherwise an
	 * i/o error results.
	 */
	case CHRECORD:
		splimp();
		while (chrempty(conn)) {
			conn->cn_sflags |= CHIWAIT;
			sleep((char *)&conn->cn_rhead,
				CHIOPRIO);
		}
		spl0();
		if ((pkt = conn->cn_rhead) == NOPKT ||
		    pkt->pk_len + 1 > UCOUNT)	/* + 1 for opcode */
			return EIO;
		else {
			int errno;
			
			if ((errno = UREAD(&pkt->pk_op, (unsigned)1)) == 0 &&
			    (errno = UREAD(pkt->pk_cdata, pkt->pk_len)) == 0) {
				splimp();
				ch_read(conn);
				spl0();
			}
			return errno;
		}
	}
	/* NOTREACHED */
}
/*
 * Raw write routine
 * Note that user programs can write to a connection
 * that has been CHIOCANSWER"'d, implying transmission of an ANS packet
 * rather than a normal packet.  This is illegal for TTY mode connections,
 * is handled in the system independent stream code for STREAM mode, and
 * is handled here for RECORD mode.
 */
chrwrite(dev)
dev_t dev;
{
#ifdef BSD42
	return ENXIO;
#else
	register struct connection *conn;
	register int errno = 0;
	
	if(minor(dev) == CHURFCMIN)
		errno = EIO;
	else if ((conn = CHFCONN(getf(u.u_arg[0]))) == NOCONN)
		errno = ENXIO;
	else
		errno = chwrite(conn, 0);
	u.u_error = errno;
#endif BSD42
}

/*
 * Return an errno or 0
 */
chwrite(conn, uio)
	register struct connection *conn;
{
	register struct packet *pkt;
	
	if (conn->cn_state == CSRFCRCVD)
		ch_accept(conn);
	switch (conn->cn_mode) {
	case CHTTY:
		/* Fall into (on RAW mode only) */
	case CHSTREAM:
		while (UCOUNT != 0) {
			int error, n;

			n = ch_swrite(conn, (char *)0, (unsigned)UCOUNT, uio, &error);
			if (error)
				return error;
			switch (n) {
			case 0:
				splimp();
				while (chtfull(conn)) {
					conn->cn_sflags |= CHOWAIT;
					sleep((caddr_t)&conn->cn_thead,
						CHIOPRIO);
				}
				spl0();
				continue;
			case CHTEMP:
				sleep((caddr_t)&lbolt, CHIOPRIO);
				continue;
			default:
				if (n < 0)
					return EIO;
			}
		}
		return 0;
	case CHRECORD:	/* One write call -> one packet */
		if (UCOUNT < 1 || UCOUNT - 1 > CHMAXDATA ||
		    conn->cn_state == CSINCT)
		    	return EIO;

		splimp();
		while (chtfull(conn)) {
			conn->cn_sflags |= CHOWAIT;
			sleep((caddr_t)&conn->cn_thead, CHIOPRIO);
		}
		spl0();
		while ((pkt = pkalloc((int)(UCOUNT - 1), 0)) == NOPKT) {
			/* block if we can't get a packet */
			sleep((caddr_t)&lbolt, CHIOPRIO);
		}
		{
			int errno = UWRITE(&pkt->pk_op, 1);

			pkt->pk_lenword = UCOUNT;
			if (errno != 0 ||
			    UCOUNT &&
			    (errno = UWRITE(pkt->pk_cdata, UCOUNT)))
				ch_free((char *)pkt);
			else {
				splimp();
				if (ch_write(conn, pkt))
					errno = EIO;
				spl0();
			}
			return errno;
		}
	}
	/* NOTREACHED */
}
/*
 * Raw ioctl routine - perform non-connection functions, otherwise call down
 * errno holds error return value until "out:"
 */
chrioctl(dev, cmd, addr, flag)
register int cmd;
caddr_t addr;
{
	int errno = 0;

	if (minor(dev) == CHURFCMIN)
		switch(cmd) {
		/*
		 * Skip the first unmatched RFC at the head of the queue
		 * and mark it so that ch_rnext will never pick it up again.
		 */
		case CHIOCRSKIP:
			splimp();
			ch_rskip();
			spl0();
			break;

#if NCHETHER > 0
		case CHIOCETHER:
			errno = cheaddr(addr);
			break;
#endif
#if NCHIL > 0
		/*
		 * Specify chaosnet address of an interlan ethernet interface.
		 */
		case CHIOCILADDR:
			{
				register struct chiladdr *ca;
				struct chiladdr chiladdr;

				if (addr == 0 ||
				    copyin(addr, (caddr_t)&chiladdr,
				    	   sizeof(chiladdr)))
				    	errno = EFAULT;
				ca = &chiladdr;
				if (errno == 0 &&
				    chilseta(ca->cil_device, ca->cil_address))
					errno = ENXIO;
			}
			break;
#endif NCHIL > 0
		case CHIOCNAME:
#ifdef BSD42
			bcopy(addr, Chmyname, CHSTATNAME);
#else
			if (copyin(addr, Chmyname, CHSTATNAME))
				errno = EFAULT;
#endif
			break;
		/*
		 * Specify my own network number.
		 */
		case CHIOCADDR:
#ifdef BSD42
			addr = *(caddr_t *)addr;
#endif
			Chmyaddr = (int)addr;
		}
#ifdef BSD42
	else {
		register struct file *fp;
		
		if (cmd != CHIOCOPEN)
			errno = ENXIO;
		else if ((fp = falloc()) == NULL)
			errno = u.u_error;
		else {
			fp->f_flag = getf(u.u_arg[0])->f_flag;
			fp->f_type = DTYPE_CHAOS;
			fp->f_ops = &chfileops;
			fp->f_data = (caddr_t)chopen((struct chopen *)addr,
						     fp->f_flag & FWRITE,
						     &errno);
			if (fp->f_data == NULL) {
				u.u_ofile[u.u_r.r_val1] = 0;
				fp->f_count = 0;
			}
		}
	}
	return errno;
#else BSD42
	else {
		struct file *fp = getf(u.u_arg[0]);
		struct connection *conn = CHFCONN(fp);
		
		if (cmd == CHIOCOPEN)
			if (conn != NOCONN)
				errno = EEXIST;
			else {
				struct chopen cho;

				if (copyin(addr, (caddr_t)&cho,
					   sizeof(struct chopen)))
					errno = EFAULT;
				else {
					addr = (caddr_t)&cho;
					conn = chopen((struct chopen *)addr,
						      fp->f_flag & FWRITE,
						      &errno);
					if (conn != NOCONN) {
						CHFCONN(fp) = conn;
						/* No seeking on this file! */
						fp->f_flag |= FMP;
					}
				}
			}
		else if (conn == NOCONN)
			errno = ENXIO;
		else
			errno = chioctl(conn, cmd, addr);
	}
	u.u_error = errno;
#endif else BSD42
}
/*
 * Returns an errno
 */
/* ARGSUSED */
chioctl(conn, cmd, addr)
register struct connection *conn;
caddr_t addr;
{
	register struct packet *pkt;
	int flag;

	switch(cmd) {
	/*
	 * Read the first packet in the read queue for a connection.
	 * This call is primarily intended for those who want to read
	 * non-data packets (which are normally ignored) like RFC
	 * (for arguments in the contact string), CLS (for error string) etc.
	 * The reader's buffer is assumed to be CHMAXDATA long.  When ioctl's
	 * change on the VAX to have data length arguments, this will be done
	 * right. An error results if there is no packet to read.
	 * No hanging is currently provided for.
	 * The normal mode of operation for reading such packets is to
	 * first do a CHIOCGSTAT call to find out whether there is a packet
	 * to read (and what kind) and then make this call - except for
	 * RFC's when you know it must be there.
	 */	
	case CHIOCPREAD:
		if ((pkt = conn->cn_rhead) == NULL)
			return ENXIO;
#ifdef BSD42
		addr = *(caddr_t *)addr;
#endif
		if (copyout((caddr_t)pkt->pk_cdata, addr, pkt->pk_len))
			return EFAULT;
		splimp();
		ch_read(conn);
		spl0();
		return 0;
	/*
	 * Change the mode of the connection.
	 * The default mode is CHSTREAM.
	 */
	case CHIOCSMODE:
#ifdef BSD42
		addr = *(caddr_t *)addr;
#endif
		switch ((int)addr) {
		case CHTTY:
#if NCHT > 0
			if (conn->cn_state == CSOPEN &&
			    conn->cn_mode != CHTTY &&
			    chttyconn(conn) == 0)
				return 0;
#endif
			return ENXIO;
		case CHSTREAM:
		case CHRECORD:
			if (conn->cn_mode == CHTTY)
				return EIO;
			conn->cn_mode = (int)addr;
			return 0;
		}
		return ENXIO;

	/* 
	 * Like (CHIOCSMODE, CHTTY) but return a tty unit to open.
	 * For servers that want to do their own "getty" work.
	 */

      case CHIOCGTTY:

#if NCHT > 0
			if (((conn->cn_state == CSOPEN) ||
			     (conn->cn_state == CSRFCRCVD)) &&
			    conn->cn_mode != CHTTY)
			  {
			    int x = chtgtty(conn);
			    *(int *)(addr) = x;
			    if (x >= 0)
			    {
			      if (conn->cn_state == CSRFCRCVD)
				ch_accept(conn);
			      conn->cn_mode = CHTTY;
			      return 0;
			    }
			  }
#endif
			return ENXIO;
		
	/*
	 * Flush the current output packet if there is one.
	 * This is only valid in stream mode.
	 * If the argument is non-zero an error is returned if the
	 * transmit window is full, otherwise we hang.
	 */
	case CHIOCFLUSH:
#ifdef BSD42
		addr = *(caddr_t *)addr;
#endif
		if (conn->cn_mode == CHSTREAM) {
			splimp();
			while ((flag = ch_sflush(conn)) == CHTEMP)
				if (addr)
					break;
				else {
					conn->cn_sflags |= CHOWAIT;
					sleep((caddr_t)&conn->cn_thead,
						CHIOPRIO);
				}
			spl0();
			return flag ? EIO : 0;
		}
		return ENXIO;
	/*
	 * Wait for all output to be acknowledged.  If addr is non-zero
	 * an EOF packet is also sent before waiting.
	 * If in stream mode, output is flushed first.
	 */
	case CHIOCOWAIT:
#ifdef BSD42
		addr = *(caddr_t *)addr;
#endif
		if (conn->cn_mode == CHSTREAM) {
			splimp();
			while ((flag = ch_sflush(conn)) == CHTEMP) {
				conn->cn_sflags |= CHOWAIT;
				sleep((caddr_t)&conn->cn_thead, CHIOPRIO);
			}
			spl0();
 			if (flag)
				return EIO;
		}
		if (addr) {
			splimp();
			while (chtfull(conn)) {
				conn->cn_sflags |= CHOWAIT;
				sleep((caddr_t)&conn->cn_thead, CHIOPRIO);
			}
			flag = ch_eof(conn);
			spl0();
			if (flag)
				return EIO;
		}
		splimp();
		while (!chtempty(conn)) {
			conn->cn_sflags |= CHOWAIT;
			sleep((caddr_t)&conn->cn_thead, CHIOPRIO);
		}
		spl0();
		return conn->cn_state != CSOPEN ? EIO : 0;
	/*
	 * Return the status of the connection in a structure supplied
	 * by the user program.
	 */
	case CHIOCGSTAT:
		{
#ifdef BSD42
#define chst ((struct chstatus *)addr)
#else
		struct chstatus chstatus;
#define chst (&chstatus)
#endif
		chst->st_fhost = conn->cn_faddr;
		chst->st_cnum = conn->cn_ltidx;
		chst->st_rwsize = conn->cn_rwsize;
		chst->st_twsize = conn->cn_twsize;
		chst->st_state = conn->cn_state;
		chst->st_cmode = conn->cn_mode;
		chst->st_oroom = conn->cn_twsize - (conn->cn_tlast - conn->cn_tacked);
		if ((pkt = conn->cn_rhead) != NOPKT) {
			chst->st_ptype = pkt->pk_op;
			chst->st_plength = pkt->pk_len;
		} else {
			chst->st_ptype = 0;
			chst->st_plength = 0;
		}
#ifndef BSD42
		if (copyout((caddr_t)chst, addr, sizeof(*chst)))
			return EFAULT;
#endif
		return 0;
		}
	/*
	 * Wait for the state of the connection to be different from
	 * the given state.
	 */
	case CHIOCSWAIT:
#ifdef BSD42
		addr = *(caddr_t *)addr;
#endif
		splimp();
		while (conn->cn_state == (int)addr)
			sleep((caddr_t)conn, CHIOPRIO);
		spl0();
		return 0;
	/*
	 * Answer an RFC.  Basically this call does nothing except
	 * setting a bit that says this connection should be of the
	 * datagram variety so that the connection automatically gets
	 * closed after the first write, whose data is immediately sent
	 * in an ANS packet.
	 */
	case CHIOCANSWER:
		flag = 0;
		splimp();
		if (conn->cn_state == CSRFCRCVD && conn->cn_mode != CHTTY)
			conn->cn_flags |= CHANSWER;
		else
			flag = EIO;
		spl0();
		return flag;
	/*
	 * Reject a RFC, giving a string (null terminated), to put in the
	 * close packet.  This call can also be used to shut down a connection
	 * prematurely giving an ascii close reason.
	 */
	case CHIOCREJECT:
		{
		register struct chreject *cr;
#ifdef BSD42
		cr = (struct chreject *)addr;
#else
		struct chreject chr;

		cr = 0;
		if (copyin(addr, (caddr_t)&chr, sizeof(chr)))
			return EFAULT;
		else
			cr = &chr;
#endif
		pkt = NOPKT;
		flag = 0;
		splimp();
		if (cr && cr->cr_length != 0 &&
		    cr->cr_length < CHMAXPKT &&
		    cr->cr_length >= 0 &&
		    (pkt = pkalloc(cr->cr_length, 0)) != NOPKT)
			if (copyin(cr->cr_reason, pkt->pk_cdata, cr->cr_length)) {
				ch_free((char *)pkt);
				flag = EFAULT;
			} else {
				pkt->pk_op = CLSOP;
				pkt->pk_lenword = cr->cr_length;
			}
		ch_close(conn, pkt, 0);
		spl0();
		return flag;
		}
	/*
	 * Accept an RFC causing the OPEN packet to be sent
	 */
	case CHIOCACCEPT:
		if (conn->cn_state == CSRFCRCVD) {
			ch_accept(conn);
			return 0;
		}
		return EIO;
	/*
 	 * Count how many bytes can be immediately read.
	 */
	case FIONREAD:
		if (conn->cn_mode != CHTTY) {
			off_t nread = 0;

			for (pkt = conn->cn_rhead; pkt != NOPKT; pkt = pkt->pk_next)
				if (ISDATOP(pkt))
					nread += pkt->pk_len;
			if (conn->cn_rhead != NOPKT)
				nread -= conn->cn_roffset;
#ifdef BSD42
			*(int *)addr = nread;
#else
			if (copyout((caddr_t)&nread, addr, sizeof(off_t)))
				return EFAULT;
#endif
			return 0;
		}
	}
	return ENXIO;
}
/*
 * Timeout routine that implements the chaosnet clock process.
 */
chtimeout()
{
	register int s = splimp();
	ch_clock();
	timeout(chtimeout, 0, 1);
	splx(s);
}


/*
 * Routines to wake up processes sleeping on connections.  These were
 * macros before, but they were getting too hairy to be expanded inline.
 * --Bruce Nemnich, 6 May 85
 */

chrwake(conn)
register struct connection *conn;
{
  if (conn->cn_sflags & CHCLOSING) {
    conn->cn_sflags &= ~CHCLOSING;
    clsconn(conn, CSCLOSED, NOPKT);
  }
  else {
    if (conn->cn_sflags & CHIWAIT) {
      conn->cn_sflags &= ~CHIWAIT;
      wakeup((char *)&conn->cn_rhead);
    }
    if (conn->cn_rsel) {
      selwakeup(conn->cn_rsel, conn->cn_flags & CHSELRCOLL);
      conn->cn_rsel = 0;
      conn->cn_flags &= ~CHSELRCOLL;
    }
    if (conn->cn_mode == CHTTY)
      chtrint(conn);
  }
}

chtwake(conn)
register struct connection *conn;
{
  if (conn->cn_sflags & CHOWAIT) {
    conn->cn_sflags &= ~CHOWAIT;
    wakeup((char *)&conn->cn_thead);
  }
  if (conn->cn_tsel) {
    selwakeup(conn->cn_tsel, conn->cn_flags & CHSELTCOLL);
    conn->cn_tsel = 0;
    conn->cn_flags &= ~CHSELTCOLL;
  }
  if (conn->cn_mode == CHTTY)
    chtxint(conn);
}


