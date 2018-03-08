#include <linux/init.h>
#include <linux/module.h>
#include <linux/device.h>
#include <linux/kernel.h>
#include <linux/types.h>
#include <linux/poll.h>
#include <linux/file.h>
#include <linux/anon_inodes.h>

#include <linux/param.h>
#include <linux/signal.h>
#include <linux/errno.h>
#include <linux/sched.h>
#include <linux/sched/signal.h>
#include <linux/proc_fs.h>
#include <linux/slab.h>
#include <asm/ioctls.h>
#include <asm/segment.h>
#include <asm/irq.h>
#include <asm/io.h>

#include "chlinux.h"

#define f_data private_data

static int		initted;	/* NCP initialization flag */
int			Rfcwaiting;	/* Someone waiting on unmatched RFC */

static struct timer_list chtimer;
static int chtimer_running;

int chread_conn(struct connection *conn, char *ubuf, int size);
int chwrite_conn(struct connection *conn, const char *ubuf, int size);
int chioctl_conn(struct connection *conn, int cmd, caddr_t addr);

void chclose_conn(struct connection *conn);

void chtimeout(unsigned long t);

static int chwaitforrfc_conn(struct connection *conn, struct packet **ppkt);

/*
 * Linux glue between generic file descriptor and chaos connection code.
 */
static ssize_t chf_read(struct file *file, char *buf, size_t size, loff_t *off);
static ssize_t chf_write(struct file *file, const char *buf, size_t size, loff_t *off);
static unsigned int chf_poll(struct file *fp, struct poll_table_struct *wait);
static long chf_ioctl(struct file *file, unsigned int cmd, unsigned long arg);
static int chf_flush (struct file *file, fl_owner_t id);
static int chf_close(struct inode *inode, struct file *file);

static struct file_operations chfileops = {
	.read =	chf_read,
	.write = chf_write,
	.poll = chf_poll,
	.unlocked_ioctl = chf_ioctl,
	.flush = chf_flush,
	.release = chf_close,
};

static ssize_t
chf_read(struct file *fp, char *ubuf, size_t size, loff_t *off)
{
	int ret;

	trace("chf_read(fp=%p)\n", fp);
	ASSERT(fp->f_op == &chfileops, "chf_read ent");
	ret = chread_conn((struct connection *)fp->f_data, ubuf, size);
	ASSERT(fp->f_op == &chfileops, "chf_read exit");
	return ret;
}

static ssize_t
chf_write(struct file *fp, const char *ubuf, size_t size, loff_t *off)
{
	int ret;

	trace("chf_write(fp=%p)\n", fp);
	ASSERT(fp->f_op == &chfileops, "chf_write ent");
	ret = chwrite_conn((struct connection *)fp->f_data, ubuf, size);
	ASSERT(fp->f_op == &chfileops, "chf_write exit");
	return ret;
}

static long
chf_ioctl(struct file *fp,
	  unsigned int cmd, unsigned long value)
{
	int ret;

	trace("chf_ioctl(fp=%p)\n", fp);
	ASSERT(fp->f_op == &chfileops, "chf_ioctl ent");
	ret = chioctl_conn((struct connection *)fp->f_data, cmd, (caddr_t)value);
	ASSERT(fp->f_op == &chfileops, "chf_ioctl exit");
	return ret;
}

static unsigned int
chf_poll(struct file *fp, struct poll_table_struct *wait)
{
	struct connection *conn = (struct connection *)fp->f_data;
	unsigned int mask;
	ASSERT(fp->f_op == &chfileops, "chf_select ent");

	mask = 0;

	poll_wait(fp, &conn->cn_read_wait, wait);
	poll_wait(fp, &conn->cn_write_wait, wait);

	cli();
	if (!chrempty(conn)) {
		mask |= POLLIN | POLLRDNORM;
	} else {
		conn->cn_sflags |= CHIWAIT;
	}
	sti();

	cli();
	if (!chtfull(conn)) {
		mask |= POLLOUT | POLLWRNORM;
	} else {
		conn->cn_sflags |= CHOWAIT;
	}
	sti();

	ASSERT(fp->f_op == &chfileops, "chf_poll exit");
	return mask;
}

static int
chf_flush(struct file *fp, fl_owner_t id)
{
	return 0;
}

static int
chf_close(struct inode *inode, struct file *fp)
{
	struct connection *conn = (struct connection *)fp->f_data;

	/*
	 * If this connection has been turned into a tty, then the
	 * tty owns it and we don't do anything.
	 */

	ASSERT(fp->f_op == &chfileops, "chf_release ent");
	if (conn && conn->cn_mode != CHTTY) {
		chclose_conn(conn);
		fp->f_data = 0;
	}

	return 0;
}

char *
chwcopy(char *from, const char *to, unsigned count, int uio, int *errorp)
{
	*errorp = verify_area(VERIFY_READ, (int *)to, count);
	if (*errorp)
		return 0;

	copy_from_user(to, from, count);
	return to + count;
}

char *
chrcopy(char *from, const char *to, unsigned count, int uio, int *errorp)
{
	*errorp = verify_area(VERIFY_WRITE, (int *)to, count);
	if (*errorp)
		return 0;

	copy_to_user(to, from, count);
	return to + count;
}

enum { NOTFULL=1, EMPTY };		/* used by chwaitforoutput_conn() */

static int
chwaitforoutput_conn(struct connection *conn, int state)
{
	int retval = 0;
	DECLARE_WAITQUEUE(wait, current);

	cli();
	add_wait_queue(&conn->cn_write_wait, &wait);
	while (1) {
		if ((state == NOTFULL && !chtfull(conn)) ||
		    (state == EMPTY && chtempty(conn)))
			break;

		conn->cn_sflags |= CHOWAIT;

		current->state = TASK_INTERRUPTIBLE;
		if (signal_pending(current)) {
			retval = -ERESTARTSYS;
			break;
		}
		schedule();
		// sleep((caddr_t)&conn->cn_thead, CHIOPRIO);
	}
	current->state = TASK_RUNNING;
	remove_wait_queue(&conn->cn_write_wait, &wait);
	sti();
	return retval;
}

static int
chwaitforflush_conn(struct connection *conn, int *pflag)
{
	int retval = 0;
	DECLARE_WAITQUEUE(wait, current);

	cli();
	add_wait_queue(&conn->cn_write_wait, &wait);
	while ((*pflag = ch_sflush(conn)) == CHTEMP) {
		conn->cn_sflags |= CHOWAIT;

		current->state = TASK_INTERRUPTIBLE;
		if (signal_pending(current)) {
			retval = -ERESTARTSYS;
			break;
		}
		schedule();
		// sleep((char *)&conn->cn_thead, CHIOPRIO);
	}
	current->state = TASK_RUNNING;
	remove_wait_queue(&conn->cn_write_wait, &wait);
	sti();
	return retval;
}

static int
chwaitfornotstate_conn(struct connection *conn, int state)
{
	int retval = 0;
	DECLARE_WAITQUEUE(wait, current);

	trace("chwaitfornotstate_conn(%p, state=%d)\n", conn, state);
	cli();
	add_wait_queue(&conn->cn_state_wait, &wait);
	while (conn->cn_state == state) {
		current->state = TASK_INTERRUPTIBLE;
		if (signal_pending(current)) {
			retval = -ERESTARTSYS;
			break;
		}
		schedule();
		// sleep((caddr_t)conn, CHIOPRIO);
	}
	current->state = TASK_RUNNING;
	remove_wait_queue(&conn->cn_state_wait, &wait);
	sti();
	trace("chwaitfornotstate_conn(state=%d) exit %d\n", state, retval);
	return retval;
}

/*
 * Access the Chaos NCP.
 * For one of two reasons:
 *	1. To perform basic NCP control functions, via minor device CHURFCMIN
 *	2. To open a connection via CHAOSMIN
 * The actual opening of the connection happens later via the CHIOCOPEN ioctl.
 */
int
chropen(struct inode * inode, struct file * file)
{
	unsigned int minor = MINOR(inode->i_rdev);
	int errno = 0;

	ch_bufalloc();
	/* initialize the NCP somewhere else? */
	if (!initted) {
		chreset();	/* Reset drivers */
		chtimer_running++;
		chtimeout(0);	/* Start clock "process" */
		initted++;
	}
	if (minor == CHURFCMIN) {
		if(Chrfcrcv == 0)
			Chrfcrcv++;
		else
			errno = -ENXIO;
	}

	return errno;
}

/*
 * Return a connection or return NULL and set *errnop to any error.
 */
struct connection *
chopen_conn(struct chopen *c, int wflag, int *errnop)
{
	struct connection *conn;
	struct packet *pkt;
	int rwsize, length;
	struct chopen cho;

	trace("chopen_conn(wflag=%d)\n", wflag);

	/* get main structure */
	*errnop = verify_area(VERIFY_READ, (int *)c, sizeof(struct chopen));
	if (*errnop)
		return NOCONN;
	copy_from_user((void *)&cho, (char *)c, sizeof(struct chopen));
	c = &cho;

	length = c->co_clength + c->co_length +
		(c->co_length ? 1 : 0);
	if (length > CHMAXPKT ||
	    c->co_clength <= 0) {
		*errnop = -E2BIG;
		return NOCONN;
	}
#if 0
	trace("c->co_length %d, c->co_clength %d, length %d\n",
	      c->co_length, c->co_clength, length);
#endif

	pkt = pkalloc(length, 0);
	if (pkt == NOPKT) {
		*errnop = -ENOBUFS;
		return NOCONN;
	}
	if (c->co_length)
		pkt->pk_cdata[c->co_clength] = ' ';

	copy_from_user(pkt->pk_cdata, c->co_contact, c->co_clength);
	if (c->co_length)
		copy_from_user(&pkt->pk_cdata[c->co_clength + 1], c->co_data,
			       c->co_length);

	rwsize = c->co_rwsize ? c->co_rwsize : CHDRWSIZE;
	SET_PH_LEN(pkt->pk_phead, length);
	conn = c->co_host ? ch_open(c->co_host, rwsize, pkt) : ch_listen(pkt, rwsize);
	if (conn == NOCONN) {
#if 0
		trace("NOCONN\n");
#endif
		*errnop = -ENXIO;
		return NOCONN;
	}
#if 0
	trace("c->co_async %d\n", c->co_async);
#endif
	trace("conn %p\n", conn);
	if (!c->co_async) {
		/*
		 * We should hang until the connection changes from
		 * its initial state.
		 * If interrupted, flush the connection.
		 */

		// current->timeout = (unsigned long) -1;

		*errnop = chwaitfornotstate_conn(conn, c->co_host ?
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
#if 0
			trace("open failed; cn_state %d\n", conn->cn_state);
#endif
			rlsconn(conn);
			*errnop = -EIO;
			return NOCONN;
		}
	}
	if (wflag)
		conn->cn_sflags |= CHFWRITE;
	conn->cn_sflags |= CHRAW;
	conn->cn_mode = CHSTREAM;
	trace("chopen_conn() done\n");
	return conn;
}

int
chrclose(struct inode * inode, struct file * file)
{
	unsigned int minor = MINOR(inode->i_rdev);

	trace("chrclose(inode=%p, fp=%p) minor=%d\n", inode, file, minor);
	if (minor == CHURFCMIN) {
		Chrfcrcv = 0;
		freelist(Chrfclist);
		Chrfclist = NOPKT;
	}
}

void
chclose_conn(conn)
	struct connection *conn;
{
	struct packet *pkt;

	trace("chclose_conn(%p)\n", conn);
	switch (conn->cn_mode) {
	case CHTTY:
		panic("chclose_conn on tty");
	case CHSTREAM:
		cli();
#if 0
		if (setjmp(&u.u_qsave)) {
			pkt = pktstr(NOPKT, "User interrupted", 16);
			if (pkt != NOPKT)
				pkt->pk_op = CLSOP;
			ch_close(conn, pkt, 0);
			goto shut;
		}
#endif
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
			chwaitforoutput_conn(conn, NOTFULL);

			if (conn->cn_state == CSOPEN ||
			    conn->cn_state == CSRFCRCVD) {
				if (conn->cn_toutput) {
					ch_sflush(conn);
					goto checkfull;
				}
				if (conn->cn_state == CSOPEN)
					(void)ch_eof(conn);
			}

			chwaitforoutput_conn(conn, EMPTY);
		} else if (conn->cn_state == CSOPEN) {
			/*
			 * If we are only reading then we should read the EOF
			 * before closing and wait for the other end to close.
			 */
			if (conn->cn_flags & CHEOFSEEN) {
				chwaitfornotstate_conn(conn, CSOPEN);
			}
		}
	recclose:
		sti();
		/* Fall into... */
	case CHRECORD:	/* Record oriented close is just sending a CLOSE */
		if (conn->cn_state == CSOPEN) {
			pkt = pkalloc(0, 0);
			if (pkt != NOPKT) {
				pkt->pk_op = CLSOP;
				SET_PH_LEN(pkt->pk_phead, 0);
			}
			ch_close(conn, pkt, 0);
		}
	}
shut:
	sti();
	ch_close(conn, NOPKT, 1);
	ch_buffree();
}
/*
 * Raw read routine.
 */
ssize_t
chrread(struct file *fp, char *buf, size_t count, loff_t *offset)
{
	struct connection *conn = (struct connection *)fp->f_data;
	unsigned int minor = iminor(file_inode(fp));
	struct packet *pkt;
	int errno = 0;

	trace("chrread(fp=%p) minor=%d\n", fp, minor);

	/* only CHURFCMIN is readable as char device */
	if(minor == CHURFCMIN) {
		if ((errno = chwaitforrfc_conn(conn, &pkt)) != 0)
			return errno;

		if (count < PH_LEN(pkt->pk_phead))
			errno = -EIO;
		else {
			copy_to_user(buf, (caddr_t)pkt->pk_cdata, PH_LEN(pkt->pk_phead));
			errno = PH_LEN(pkt->pk_phead);
		}
	} else
		errno = -ENXIO;

	return errno;
}

static int
chwaitforrfc_conn(struct connection *conn, struct packet **ppkt)
{
	int retval = 0;
	DECLARE_WAITQUEUE(wait, current);

	cli();
	add_wait_queue(&Rfc_wait_queue, &wait);
	while (1) {
		if ((*ppkt = ch_rnext()) != NOPKT)
			break;
		Rfcwaiting++;
		current->state = TASK_INTERRUPTIBLE;
		if (signal_pending(current)) {
			retval = -ERESTARTSYS;
			break;
		}
		// sleep((caddr_t)&Chrfclist, CHIOPRIO);
		schedule();
	}
	current->state = TASK_RUNNING;
	remove_wait_queue(&Rfc_wait_queue, &wait);
	sti();
	return retval;
}

static int
chwaitfordata_conn(struct connection *conn)
{
	int retval = 0;
	DECLARE_WAITQUEUE(wait, current);

	trace("chwaitfordata_conn(%p)\n", conn);
	cli();
	add_wait_queue(&conn->cn_read_wait, &wait);
	while (chrempty(conn)) {
		conn->cn_sflags |= CHIWAIT;

		current->state = TASK_INTERRUPTIBLE;
		if (signal_pending(current)) {
			retval = -ERESTARTSYS;
			break;
		}
		schedule();
		// sleep((char *)&conn->cn_rhead, CHIOPRIO);
	}
	current->state = TASK_RUNNING;
	remove_wait_queue(&conn->cn_read_wait, &wait);
	sti();
	return retval;
}

/*
 * Return an errno on error
 */
int
chread_conn(struct connection *conn, char *ubuf, int size)
{
	struct packet *pkt;
	int count, errno;

	trace("chread(%p)\n", conn);
	switch (conn->cn_mode) {
	case CHTTY:
		return -ENXIO;
	case CHSTREAM:
		trace("chread() CHSTREAM\n");
		if (conn->cn_state == CSRFCRCVD)
			ch_accept(conn);
		for (count = size; size != 0; ) {
			int error, n;

			n = ch_sread(conn, ubuf, (unsigned)size, 0, &error);
			if (error)
				return error;
			switch (n) {
			case 0:	/* No data to read */
				if (count != size)
					return 0;
				if ((errno = chwaitfordata_conn(conn)) != 0)
					return errno;
				continue;
			case CHEOF:
				return 0;
			default:
				if (n < 0)
					return -EIO;
				return n;
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
		trace("chread() CHRECORD size %d\n", size);
		if ((errno = chwaitfordata_conn(conn)) != 0)
			return errno;
#ifdef DEBUG_CHAOS
		if ((pkt = conn->cn_rhead) != NOPKT)
			trace("chread() CHRECORD pk_len %d, size %d\n",
			      PH_LEN(pkt->pk_phead), size);
#endif
		if ((pkt = conn->cn_rhead) == NOPKT ||
		    PH_LEN(pkt->pk_phead) + 1 > size)	/* + 1 for opcode */
			return -EIO;
		else {
			int errno;

			errno = verify_area(VERIFY_WRITE, (int *)ubuf,
					    1 + PH_LEN(pkt->pk_phead));
			if (errno == 0) {
				copy_to_user(ubuf, &pkt->pk_op, 1);
				copy_to_user(ubuf+1, pkt->pk_cdata,
					     PH_LEN(pkt->pk_phead));
				errno = PH_LEN(pkt->pk_phead) + 1;

				cli();
				ch_read(conn);
				sti();
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
ssize_t
chrwrite(struct file *file,
	 const char *buf, size_t count, loff_t *offset)
{
	unsigned int minor = iminor(file_inode(file));
	trace("chrwrite(fp=%p) minor=%d\n", file, minor);
	return -ENXIO;
}

/*
 * Return an errno or 0
 */
int
chwrite_conn(struct connection *conn, const char *ubuf, int size)
{
	struct packet *pkt;
	int errno;

	trace("chwrite_conn(%p)\n", conn);
	if (conn->cn_state == CSRFCRCVD)
		ch_accept(conn);
	switch (conn->cn_mode) {
	case CHTTY:
		/* Fall into (on RAW mode only) */
	case CHSTREAM:
		while (size != 0) {
			int n;

			n = ch_swrite(conn, ubuf, (unsigned)size, 0, &errno);
			if (errno)
				return errno;
			switch (n) {
			case 0:
			case CHTEMP:	/* output window is full */
				if ((errno = chwaitforoutput_conn(conn, NOTFULL)) != 0)
					return errno;
				continue;
			default:
				if (n < 0)
					return -EIO;
				return n;
			}
		}
		return 0;
	case CHRECORD:	/* One write call -> one packet */
		if (size < 1 || size - 1 > CHMAXDATA ||
		    conn->cn_state == CSINCT)
			return -EIO;

		trace("chwrite_conn() tlast %d, tacked %d, twsize %d\n",
		      conn->cn_tlast, conn->cn_tacked, conn->cn_twsize);

		if ((errno = chwaitforoutput_conn(conn, NOTFULL)) != 0)
			return errno;

#if 0
		while ((pkt = pkalloc((int)(size - 1), 0)) == NOPKT) {
			/* block if we can't get a packet */
			sleep((caddr_t)&lbolt, CHIOPRIO);
		}
#else
		if ((pkt = pkalloc((int)(size - 1), 0)) == NOPKT)
			return -EIO;
#endif

		copy_from_user(&pkt->pk_op, ubuf, 1);
		SET_PH_LEN(pkt->pk_phead, size-1);
		if (size)
			copy_from_user(pkt->pk_cdata, ubuf+1, size-1);

		cli();
		if (ch_write(conn, pkt))
			errno = -EIO;
		else
			errno = size;
		sti();

		return errno;
	}
	/* NOTREACHED */
}
/*
 * Raw ioctl routine - perform non-connection functions, otherwise call down
 * errno holds error return value until "out:"
 */
long
chrioctl(struct file *fp,
	 unsigned int cmd, unsigned long addr)
{
	unsigned int minor = iminor(file_inode(fp));
	int errno = 0;

	trace("chrioctl(fp=%p) minor=%d\n", fp, minor);
	if (minor == CHURFCMIN) {
		switch(cmd) {
		/*
		 * Skip the first unmatched RFC at the head of the queue
		 * and mark it so that ch_rnext will never pick it up again.
		 */
		case CHIOCRSKIP:
			cli();
			ch_rskip();
			sti();
			break;

		case CHIOCETHER:
			errno = cheaddr(addr);
			break;

		case CHIOCNAME:
			errno = verify_area(VERIFY_READ, (int *)addr,
					    CHSTATNAME);
			if (errno == 0)
				copy_from_user(Chmyname, (char *)addr,
					       CHSTATNAME);
			break;
		/*
		 * Specify my own network number.
		 */
		case CHIOCADDR:
			Chmyaddr = addr;
			break;
		}
	} else {
		int fd;
		struct connection *chdata;

		if (cmd != CHIOCOPEN)
			return -ENXIO;

		chdata = chopen_conn((struct chopen *)addr, fp->f_flags & (O_WRONLY|O_RDWR), &errno);
		if (chdata == NULL)
			printk("chdata == NULL!!!\n");

		/* get a file descriptor suitable for return to the user */
		fd = anon_inode_getfd("chaos", &chfileops, chdata, fp->f_flags);
		if (IS_ERR(fd)) {
			trace("no file\n");
			return -ENFILE;
		}
		errno = fd;
	}
	return errno;
}
/*
 * Returns an errno
 */
int
chioctl_conn(conn, cmd, addr)
	struct connection *conn;
	int cmd;
	caddr_t addr;
{
	struct packet *pkt;
	int flag, retval;

	trace("chioctl_conn(%p)\n", conn);
	switch(cmd) {
	/*
	 * Read the first packet in the read queue for a connection.
	 * This call is primarily intended for those who want to read
	 * non-data packets (which are normally ignored) like RFC
	 * (for arguments in the contact string), CLS (for error string) etc.
	 * The reader's buffer is assumed to be CHMAXDATA long.
	 *
	 * An error results if there is no packet to read.
	 * No hanging is currently provided for.
	 * The normal mode of operation for reading such packets is to
	 * first do a CHIOCGSTAT call to find out whether there is a packet
	 * to read (and what kind) and then make this call - except for
	 * RFC's when you know it must be there.
	 */
	case CHIOCPREAD:
		trace("CHIOCPREAD\n");
		if ((pkt = conn->cn_rhead) == NULL)
			return -ENXIO;

		retval = verify_area(VERIFY_WRITE, (int *)addr, PH_LEN(pkt->pk_phead));
		if (retval)
			return retval;

		copy_to_user((int *)addr, (caddr_t)pkt->pk_cdata, PH_LEN(pkt->pk_phead));

		cli();
		ch_read(conn);
		sti();
		return 0;
	/*
	 * Change the mode of the connection.
	 * The default mode is CHSTREAM.
	 */
	case CHIOCSMODE:
		trace("CHIOCSMODE conn %p\n", conn);
		switch ((int)addr) {
		case CHTTY:
#ifdef NCHT
			if (conn->cn_state == CSOPEN &&
			    conn->cn_mode != CHTTY &&
			    chttyconn(conn) == 0)
				return 0;
#endif
			return -ENXIO;
		case CHSTREAM:
		case CHRECORD:
			if (conn->cn_mode == CHTTY)
				return -EIO;
			conn->cn_mode = (int)addr;
			return 0;
		}
		return -ENXIO;

	/*
	 * Like (CHIOCSMODE, CHTTY) but return a tty unit to open.
	 * For servers that want to do their own "getty" work.
	 */
	case CHIOCGTTY:
		trace("CHIOCGTTY\n");

#ifdef NCHT
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
		return -ENXIO;

	/*
	 * Flush the current output packet if there is one.
	 * This is only valid in stream mode.
	 * If the argument is non-zero an error is returned if the
	 * transmit window is full, otherwise we hang.
	 */
	case CHIOCFLUSH:
		trace("CHIOCFLUSH\n");
		if (conn->cn_mode == CHSTREAM) {
			if (addr) {
				flag = ch_sflush(conn);
			} else {
				if ((retval = chwaitforflush_conn(conn, &flag)) != 0)
					return retval;
			}
			return flag ? -EIO : 0;
		}
		return -ENXIO;
	/*
	 * Wait for all output to be acknowledged.  If addr is non-zero
	 * an EOF packet is also sent before waiting.
	 * If in stream mode, output is flushed first.
	 */
	case CHIOCOWAIT:
		trace("CHIOCOWAIT\n");
		if (conn->cn_mode == CHSTREAM) {
			if ((retval = chwaitforflush_conn(conn, &flag)) != 0)
				return retval;
			if (flag)
				return -EIO;
		}
		if (addr) {
			if ((retval = chwaitforoutput_conn(conn, NOTFULL)) != 0)
				return retval;
			cli();
			flag = ch_eof(conn);
			sti();
			if (flag)
				return -EIO;
		}
		if ((retval = chwaitforoutput_conn(conn, EMPTY)) != 0)
			return retval;
		return conn->cn_state != CSOPEN ? EIO : 0;
	/*
	 * Return the status of the connection in a structure supplied
	 * by the user program.
	 */
	case CHIOCGSTAT:
		trace("CHIOCGSTAT conn %p\n", conn);
		if (conn == 0)
			return -ENXIO;

		{
			struct chstatus chst;
			int errno;

			chst.st_fhost = CH_ADDR_SHORT(conn->cn_faddr);
			chst.st_cnum = conn->cn_ltidx;
			chst.st_rwsize = SHORT_TO_LE(conn->cn_rwsize);
			chst.st_twsize = SHORT_TO_LE(conn->cn_twsize);
			chst.st_state = conn->cn_state;
			chst.st_cmode = conn->cn_mode;
			chst.st_oroom = SHORT_TO_LE(conn->cn_twsize - (conn->cn_tlast - conn->cn_tacked));
			if ((pkt = conn->cn_rhead) != NOPKT) {
				trace("pkt %p\n", pkt);
				chst.st_ptype = pkt->pk_op;
				chst.st_plength = SHORT_TO_LE(PH_LEN(pkt->pk_phead));
			} else {
				chst.st_ptype = 0;
				chst.st_plength = 0;
			}

			errno = verify_area(VERIFY_WRITE, (int *)addr,
					    sizeof(struct chstatus));
			if (errno)
				return errno;

			copy_to_user((int *)addr, (caddr_t)&chst,
				     sizeof(struct chstatus));

			trace("done\n");
			return 0;
		}
	/*
	 * Wait for the state of the connection to be different from
	 * the given state.
	 */
	case CHIOCSWAIT:
		trace("CHIOCSWAIT\n");
		if ((retval = chwaitfornotstate_conn(conn, (int)addr)) != 0)
			return retval;
		return 0;
	/*
	 * Answer an RFC.  Basically this call does nothing except
	 * setting a bit that says this connection should be of the
	 * datagram variety so that the connection automatically gets
	 * closed after the first write, whose data is immediately sent
	 * in an ANS packet.
	 */
	case CHIOCANSWER:
		trace("CHIOCANSWER\n");
		flag = 0;
		cli();
		if (conn->cn_state == CSRFCRCVD && conn->cn_mode != CHTTY)
			conn->cn_flags |= CHANSWER;
		else
			flag = EIO;
		sti();
		return flag;
	/*
	 * Reject a RFC, giving a string (null terminated), to put in the
	 * close packet.  This call can also be used to shut down a connection
	 * prematurely giving an ascii close reason.
	 */
	case CHIOCREJECT:
		trace("CHIOCREJECT\n");
		{
			struct chreject cr;

			retval = verify_area(VERIFY_READ, (int *)addr,
					     sizeof(struct chreject));
			if (retval)
				return retval;

			copy_from_user(&cr, (int *)addr, sizeof(struct chreject));

			pkt = NOPKT;
			flag = 0;
			cli();
			if (cr.cr_length != 0 &&
			    cr.cr_length < CHMAXPKT &&
			    cr.cr_length >= 0 &&
			    (pkt = pkalloc(cr.cr_length, 0)) != NOPKT)
			{
				retval = verify_area(VERIFY_READ, (int *)cr.cr_reason,
						     cr.cr_length);
				if (retval) {
					ch_free((char *)pkt);
					return retval;
				}

				copy_from_user(pkt->pk_cdata, cr.cr_reason,
					       cr.cr_length);

				pkt->pk_op = CLSOP;
				SET_PH_LEN(pkt->pk_phead, cr.cr_length);
			}
			ch_close(conn, pkt, 0);
			sti();
			return flag;
		}
	/*
	 * Accept an RFC causing the OPEN packet to be sent
	 */
	case CHIOCACCEPT:
		trace("CHIOCACCEPT conn %p\n", conn);
		if (conn->cn_state == CSRFCRCVD) {
			ch_accept(conn);
			return 0;
		}
		return -EIO;
	/*
	 * Count how many bytes can be immediately read.
	 */
	case FIONREAD:
		trace("FIONREAD\n");
		if (conn->cn_mode != CHTTY) {
			off_t nread = 0;
			int nr, errno;

			for (pkt = conn->cn_rhead; pkt != NOPKT; pkt = pkt->pk_next)
				if (ISDATOP(pkt))
					nread += PH_LEN(pkt->pk_phead);
			if (conn->cn_rhead != NOPKT)
				nread -= conn->cn_roffset;

			errno = verify_area(VERIFY_WRITE, (int *)addr,
					    sizeof(int));
			if (errno)
				return errno;

			nr = nread;
			copy_to_user((int *)addr, (caddr_t)&nr, sizeof(int));
			trace("FIONREAD returns %d bytes\n", nr);
			return 0;
		}
	}
	return -ENXIO;
}

/*
 * Timeout routine that implements the chaosnet clock process.
 * (called periodically)
 */
void
chtimeout(unsigned long t)
{
	cli();
	ch_clock();

	if (chtimer_running) {
		if (chtimer.entry.next)
			del_timer(&chtimer);

		chtimer.expires = jiffies + 1;
		chtimer.function = chtimeout;
		chtimer.data = (unsigned long)0;
		add_timer(&chtimer);
	}

	sti();
}

void
chtimeout_stop(void)
{
	cli();
	chtimer_running = 0;
	if (chtimer.entry.next)
		del_timer(&chtimer);
	sti();
}


/*
 * Routines to wake up processes sleeping on connections.  These were
 * macros before, but they were getting too hairy to be expanded inline.
 * --Bruce Nemnich, 6 May 85
 */

void
chrwake(struct connection *conn)
{
	if (conn->cn_sflags & CHCLOSING) {
		conn->cn_sflags &= ~CHCLOSING;
		clsconn(conn, CSCLOSED, NOPKT);
	} else {
		if (conn->cn_sflags & CHIWAIT) {
			conn->cn_sflags &= ~CHIWAIT;
			wake_up_interruptible(&conn->cn_read_wait);
			// wakeup((char *)&conn->cn_rhead);
		}
		if (conn->cn_mode == CHTTY)
			chtrint(conn);
	}
}

void
chtwake(struct connection *conn)
{
	if (conn->cn_sflags & CHOWAIT) {
		conn->cn_sflags &= ~CHOWAIT;
		wake_up_interruptible(&conn->cn_write_wait);
		// wakeup((char *)&conn->cn_thead);
	}
	if (conn->cn_mode == CHTTY)
		chtxint(conn);
}
