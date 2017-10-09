/*
 *	Server SUPDUP program.
 *	The standard input is initially a connection which has been
 *	listened to, but neither accepted nor rejected.
 *	First pass DCP, current code with cursor hacking JEK.
 */

#include <stdio.h>
#include <ctype.h>
#include <errno.h>
#include <signal.h>
#include <sgtty.h>
#include <chaos/user.h>

/*
 * Characters output from user programs are checked for \b, \r, and \n.
 * These are translated into SUPDUP cursor movement.  Thus this disallows
 * the use of these 3 SAIL characters.
 * SUPDUP display codes are passed through, but looked at to determine
 * the current cursor position.  This is necessary for simulating \b, \n, \r
 * and the TDBS and TDUP fake SUPDUP codes supplied in the termcap entries.
 * Input from the network is checked for ITP escapes, mainly for bucky
 * bits and terminal location string.
 */

#define TRUE -1
#define FALSE 0

/*
 * Since various parts of UNIX just can't hack the 0200 bit, we define
 * a fake lead-in escape - 0177, to avoid a sail graphic that is used
 * sometimes...
 */
#define SUPDUP_ESCAPE 0177

/* ITP bits (from the 12-bit character set */
#define ITP_ASCII	00177	/* ascii part of character */
#define ITP_CTL		00200	/* CONTROL key depressed */
#define ITP_MTA		00400	/* META key depressed */
#define ITP_TOP		04000	/* TOP key depressed */

#define ITP_QUOTE	034	/* quoting character */
#define ITP_CHAR(char1,char2) (((char1 & 037) << 7) + char2)

#define ASCII_CTL_MASK ~(0177-037)
#define ASCII_ESCAPE	033
#define ASCII_PART(char) (char & ITP_ASCII)

#define ITP_ESCAPE 	0300
#define ITP_LOGOUT	0301
#define ITP_LOCATION	0302

#define TDMOV	0200
#define TDMV1	0201
#define TDEOF	0202
#define TDEOL	0203
#define TDDLF	0204
#define TDCRL	0207
#define TDNOP	0210
#define TDORS	0214
#define TDQOT	0215
#define TDFS	0216
#define TDMV0	0217
#define TDCLR	0220
#define TDBEL	0221
#define TDILP	0223
#define TDDLP	0224
#define TDICP	0225
#define TDDCP	0226
#define TDBOW	0227
#define TDRST	0230
#define TDGRF	0231

#define TDBS	0211	/* Interpreted locally, not in all supdup's */
#define	TDUP	0213	/* Interpreted locally, not in supdup at all */
int currcol, currline;	/* Current cursor position */

/* These variables are set at initial connection time */
char ttyopt[6];
#define TOERS	(ttyopt[0] & 4)		/* Terminal can erase */
#define TOMVB	(ttyopt[0] & 1)		/* can move backwards */
#define TOOVR	(ttyopt[1] & 010)	/* Over printing */
#define TOMVU	(ttyopt[1] & 4)		/* can move up */
#define TOLID	(ttyopt[2] & 2)		/* Line insert/delete */
#define TOCID	(ttyopt[2] & 1)
#define TOSA1	(ttyopt[1] & 020)	/* send SAIL characters direct */
#define TOMOR	(ttyopt[1] & 2)		/* Do more processing */
#define TOROL	(ttyopt[1] & 1)		/* Do scrolling */
#define TPPRN	(ttyopt[4] & 2)		/* Swap parans and brackets */
short ttyrol;	/* How much the terminal scrolls by */
short tcmxv;	/* Number of lines */
short tcmxh;	/* Number of columns */

int ppipe[2];	/* Pipe from net reader to net writer for parameters */

int conn = -1;			/* chaos connection */
int him;			/* process id of the other process */

struct chpacket pkt;
char *ptr = pkt.cp_data;
int cnt = 0;
char pty[20];


main(argc, argv)
int argc; char *argv[];
{
	int i, c, f, pid;

	pid = getpid();
	if (pipe(ppipe) < 0) {
		ioctl(0, CHIOCREJECT, "Can't create pipe for second process");
		return(1);
	}
	for (c = 'p'; c < 'w'; c++)
		for (i = 0; i < 16; i++) {
			sprintf(pty, "/dev/pty%c%x", c, i);
			if ((f = open(pty, 2)) >= 0)
				goto win;
		}
	ioctl(0, CHIOCREJECT, "?All PTYs in use");
	return(1);
win:
	ioctl(0, CHIOCACCEPT, 0);
	conn = dup(0);
	close(0);
	close(1);
	dup(f);
	dup(f);		/* PTY now standard output/input */
	close(f);
	ioctl(conn, CHIOCSMODE, CHRECORD);
	read(conn, &pkt, sizeof pkt);	/* Read and discard the RFC */
	him = fork();
	if (him == 0) {
		him = pid;
		iparams();	/* get the initial tty parameters */
		rd();
		shut();
	}
	gparams();		/* receive parameters */
	greet();		/* send greeting */
	wr();
	ioctl(conn, CHIOCOWAIT, 1);
	shut();
}
shut()
{
	kill(him, SIGKILL); /* kill HIM !! */
	exit(1);
}
/*
 * Read the parameters from the net and send them to the net writer.
 */
iparams()
{
	char temp[6];

	rd36(0);	/* Read and flush count */
	rd36(0);	/* Read and flush terminal type - TCTYP */
	rd36(ttyopt);	/* Read TTYOPT word */
	rd36(temp);	/* Read TCMXV word - number of lines */
	tcmxv = (temp[5] & 0377) | ((temp[4] & 0377) << 6);
	rd36(temp);	/* Read TCMXH word - number of columns */
	tcmxh = (temp[5] & 0377) | ((temp[4] & 0377) << 6);
	rd36(temp);	/* Read TTYROL word - scroll amount */
	ttyrol = temp[5] & 077;
	rd36(0);	/* Read SMARTS word - graphics capabilities */
	rd36(0);	/* Read ISPEED word */
	rd36(0);	/* Read OSPEED word */
	if (write(ppipe[1], ttyopt, 6) != 6 ||
	    write(ppipe[1], &tcmxv, sizeof(tcmxv)) != sizeof(tcmxv) ||
	    write(ppipe[1], &tcmxh, sizeof(tcmxh)) != sizeof(tcmxh) ||
	    write(ppipe[1], &ttyrol, sizeof(ttyrol)) != sizeof(ttyrol))
		shut();
}

rd36(cp)
register char *cp;
{
	register int i;

	for (i = 0; i < 6; i++)
		if (cp)
			*cp++ = rdconn();
		else
			rdconn();
}
gparams()
{
	char sdfile[20];
	char termbuf[BUFSIZ];
	int f;

	if (read(ppipe[0], ttyopt, 6) != 6 ||
	    read(ppipe[0], &tcmxv, sizeof(tcmxv)) != sizeof(tcmxv) ||
	    read(ppipe[0], &tcmxh, sizeof(tcmxh)) != sizeof(tcmxh) ||
	    read(ppipe[0], &ttyrol, sizeof(ttyrol)) != sizeof(ttyrol))
		shut();

	sprintf(termbuf, "SD|supdup|SUPDUP virtual terminal:co#%d:li#%d:",
		tcmxh, tcmxv);
	strcat(termbuf, "am:vb=\\177\\23:nd=\\177\\20:");
	strcat(termbuf, "cl=\\177\\22:so=\\177\\31:se=\\177\\32:pt:");
	if (TOERS)
		strcat(termbuf, "ce=\\177\\05:ec=\\177\\06:cd=\\177\\04:");
	if (TOMVB)
		strcat(termbuf, "bs:");
	if (TOOVR)
		strcat(termbuf, "os:");
	if (TOMVU)
		strcat(termbuf, "up=\\177\\15:cm=\\177\\21%.%.:");
	if (TOLID)
		strcat(termbuf, "al=\\177\\25\\01:dl=\\177\\26\\01:");
	if (TOCID) {
		strcat(termbuf, "mi:im=:ei=:ic=\\177\\27\\01:");
		strcat(termbuf, "dc=\\177\\30:dm=:ed=:");
	}
	if (!TOROL)
		strcat(termbuf, "ns:");
	sprintf(sdfile, "/tmp/sd-tty%s", rindex(pty, '/') + 4);
	if ((f = creat(sdfile, 0644)) >= 0) {
		write(f, termbuf, strlen(termbuf));
		close(f);
	}
}
greet()
{
	register char *cp;
	char buf[100];

	sprintf(buf, "\r\nUNIX SUPDUP Server (%d lines, %d columns)\r\n%c",
		tcmxv, tcmxh, TDNOP);
	for (cp = buf; *cp; wrconn(*cp++ & 0377))
		;
	chflush(DATOP);
	sleep(2);
	wrsup(TDCLR);
}

/* Flush the packet on alarm timers */
timer()
{
	signal(SIGALRM, timer);
	chflush(DATOP);
}

/* read from the user's tty and write to the network connection */
wr()
{
	register int c;
	struct chstatus foo;
	long waiting;
	extern int errno;

	signal(SIGALRM, timer);
loop:
	while (alarm(1), errno = 0, (c = getchar()) != EOF) {
		alarm(0);	/* disable timer after the read */
		wrsup(c);
		/****************************************
		 * DEPENDS ON UNIX STDIO IMPLEMENTATION *
		 ****************************************/
		if (stdin->_cnt <= 0 &&
		    ioctl(fileno(stdin), FIONREAD, &waiting) >= 0 &&
		    waiting == 0 &&
		    ioctl(conn, CHIOCGSTAT, &foo) >= 0 &&
		    foo.st_oroom > 0)
			chflush(DATOP);
	}
	if (errno == EINTR) {
		clearerr(stdin);
		goto loop;
	}
	return(0);
}

/* write a character to the connection */
wrconn(c)
int c;
{
	*ptr++ = c;
	if (--cnt <= 0)
		chflush(DATOP);
}

/* flush the current packet to the network with specified opcode */
chflush(op)
int op;
{
	if (ptr == pkt.cp_data)
		return;
	pkt.cp_op = op;
	write(conn, &pkt, ptr - pkt.cp_data + 1);
	ptr = pkt.cp_data;
	cnt = sizeof(pkt.cp_data);
}

/* read packets from the chaos net and stuff them into the unix pty */

rd()
{
	register int c;

	while(1) {
		if (emptyconn()) 
			fflush(stdout);
		switch (c = rdconn()) {
		case ITP_ESCAPE:
			switch (rdconn()) {
			case ITP_LOGOUT:
				shut();
				break;
			case ITP_LOCATION:
				while (rdconn())
					;
			}
			break;
		case ITP_QUOTE:
			if ((c = rdconn()) != ITP_QUOTE)
				c = ITP_CHAR(c, rdconn());
			if (c & ITP_CTL)
				c = c & ASCII_CTL_MASK;
			if (c & ITP_MTA)
				putchar(ASCII_ESCAPE);
		default:
			putchar(ASCII_PART(c));
		}
	}
}

/* read a character from the network connection */
rdconn()
{
	char c;
	if (cnt-- > 0)
		return(*ptr++ & 0377);
	if ((cnt = read(conn, &pkt, sizeof(pkt))) <= 0)
		shut();
	switch (pkt.cp_op & 0377) {
	case DATOP:
		break;
	case CLSOP:
	case EOFOP:
	default:
		shut();
	}
	cnt--;			/* skip past the opcode */
	ptr = pkt.cp_data+1;
	cnt--;
	return(pkt.cp_data[0] & 0377);
}

/* return true if nothing can be read form the connection */
emptyconn()
{
	long waiting;

	if (cnt <= 0 ||
	    ioctl(conn, FIONREAD, &waiting) < 0 ||
	    waiting == 0)
	 	return(TRUE);
	return(FALSE);
}

wrsup(c)
register int c;
{
	register int i, j;
	static int state, count, bytes[4], wrapped;

top:
	switch (state) {
	case SUPDUP_ESCAPE:
		c += 0176;
		state = 0;
	case 0:
		count = 0;
		if (c == SUPDUP_ESCAPE) {
			state = SUPDUP_ESCAPE;
			return;
		}
		if (c < 0200) {
			switch (c) {
			case '\7':
				wrconn(TDBEL);
				break;
			case '\b':
				if (currcol) {
					currcol--;
					wcurpos();
				}
				break;
			case '\r':
				if (currcol) {
					currcol = 0;
					wcurpos();
				}
				break;
			case '\n':
				if (wrapped) {
					c = TDCRL;
					goto top;
				}					
				if (currline != tcmxv - 1)
					currline++;
				else
					goto scroll;
				wcurpos();
				break;
			case '\t':
				do {
					currcol++;
				} while (currcol & 7);
				if (currcol >= tcmxh) {
					i = currcol - tcmxh;
					wrsup('\r');
					wrsup('\n');
					currcol = i;
					while (i--)
						wrconn(TDFS);
				} else
					wcurpos();
				break;
			default:
				if (c <= 037 && !(ttyopt[1] & TOSA1)) {
					wrsup('^');
					c |= 0100;
				}
				wrconn(c);
				currcol++;
				if (currcol >= tcmxh) {
					wrsup('\r');
					wrsup('\n');
				}
			}
			return;
		} else switch (c) {
		case TDUP:
			if (currline) {
				currline--;
				wcurpos();
			}
			wrapped = 0;
			return;
		case TDBS:
			if (currcol) {
				currcol--;
				wcurpos();
			}
			return;
		case TDMOV:
		case TDMV1:
		case TDMV0:
		case TDILP:
		case TDDLP:
		case TDICP:
		case TDDCP:
			state = c;
		case TDNOP:
		case TDEOF:
		case TDEOL:
		case TDDLF:
		case TDORS:
		case TDBEL:
		case TDBOW:
		case TDRST:
			break;
		case TDCRL:
			currcol = 0;
			if (currline != tcmxv - 1)
				currline++;
			else {
		scroll:		if (TOROL && ttyrol) {
					if (ttyrol > 1)
						currline -= ttyrol - 1;
					wrconn(TDCRL);
					if (currcol)
						wcurpos();
				} else {
					wrapped = 1;
					currline = 0;
					i = currcol;
					currcol = 0;
					wcurpos();
					if (TOERS)
						wrconn(TDEOL);
					if (i) {
						currcol = i;
						wcurpos();
					}
				}
				return;
			}
						
			break;
		case TDFS:
			currcol++;	/* Check for end of line? */
			break;
		case TDCLR:
			wrapped = 0;
			currcol = currline = 0;
			break;
		case TDQOT:		/* A gross hack to make getty work! */
		default:
			c &= 0177;	/* Send out the ascii */
			goto top;
		}
		break;
	case TDMOV:
		if (count == 4) {
			currline = bytes[2];
			currcol = bytes[3];
			state = 0;
			wrapped = 0;
			goto top;
		} else
			bytes[count++] = c;
		break;
	case TDMV0:
	case TDMV1:
		if (count == 2) {
			currline = bytes[0];
			currcol = bytes[1];
			state = 0;
			wrapped = 0;
			goto top;
		} else
			bytes[count++] = c;
		break;
	case TDQOT:
	case TDILP:
	case TDDLP:
	case TDICP:
	case TDDCP:
		state = 0;
		break;
	}
	wrconn(c);
}
wcurpos()
{
	wrconn(TDMV0);
	wrconn(currline);
	wrconn(currcol);
}
