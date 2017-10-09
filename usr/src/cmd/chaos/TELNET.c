/*
 *	Server TELNET program.
 *	Usable on the Chaosnet (or the Arpanet via a Chaos/Arpa gateway),
 *	although it should be easy to change for any other protocol supporting
 *	reliable bytestreams.
 *	The standard input is initially a connection which has been
 *	listened to, but neither accepted nor rejected.
 *	Author: T. J. Teixeira <TJT@MIT-XX>
 *
 * 12/29/81 tony -- Check for LOS packets, and clear errno before doing
 *			read on connection.
 */

#include <stdio.h>
#include <ctype.h>
#include <errno.h>
#include <signal.h>
#include <sgtty.h>
#include <chaos/user.h>

#define ctrl(x) ((x)&037)

/*
 * telnet protocol command characters
 */
#define SE	240
#define NOP	241
#define DMARK	242
#define BRK	243
#define IP	244
#define AO	245
#define AYT	246
#define EC	247
#define EL	248
#define GA	249
#define SB	250
#define WILL	251
#define WONT	252
#define DO	253
#define DONT	254
#define IAC	255

/*
 * telnet option codes
 */
#define TN_TRANSMIT_BINARY 0
#define TN_ECHO 1
#define TN_RCP 2
#define TN_SUPPRESS_GO_AHEAD 3
#define TN_NAME 4
#define TN_STATUS 5
#define TN_TIMING_MARK 6
#define TN_RCTE 7
#define TN_NAOL 8
#define TN_NAOP 9
#define TN_NAOCRD 10
#define TN_NAOHTS 11
#define TN_NAHTD 12
#define TN_NAOFFD 13
#define TN_NAOVTS 14
#define TN_NAVTD 15
#define TN_NAOLFD 16
#define TN_EXTEND_ASCII 17
#define TN_EXOPL 255

char *optnames[] = {
	"TRANSMIT BINARY",
	"ECHO",
	"RCP",
	"SUPPRESS GO AHEAD",
	"NAME",
	"STATUS",
	"TIMING MARK",
	"RCTE",
	"NAOL",
	"NAOP",
	"NAOCRD",
	"NAOHTS",
	"NAHTD",
	"NAOFFD",
	"NAOVTS",
	"NAVTD",
	"NAOLFD",
	"EXTEND ASCII",
	"EXOPL",
};

char *optcmd[] = {
	"SE",
	"NOP",
	"DMARK",
	"BRK",
	"IP",
	"AO",
	"AYT",
	"EC",
	"EL",
	"GA",
	"SB",
	"WILL",
	"WONT",
	"DO",
	"DONT",
	"IAC",
};

char hisopts[256], myopts[256];

int bye();

struct host_entry *host;
char contact[80];

int conn = -1;	/* connection */
int fin = -1;	/* input from other process */
int fout = -1;	/* output to other process */
int pid;	/* process id of write process */
int fk;		/* process id of read process */
int isopen;	/* connection open */
int echomode;
int goahead = 1;

/* special characters */
int t_erase;	/* EC */
int t_kill;	/* EL */
int t_intrc;	/* IP */
int t_flushc;	/* AO */

/*
 * commands between processes
 */
#define IPC_MODE 1
#define IPC_CLOSED 2
#define IPC_GA 3
#define IPC_DUMMY 4
#define IPC_XMIT_BIN 5

char errbuf[BUFSIZ];

main(argc, argv)
char *argv[];
{
	register int i, c, f;
	struct sgttyb stbuf;
	struct chpacket junk;
	char buf[BUFSIZ];
	char pty[20];
	int ipc();
	int tord[2], towr[2];
#ifdef DEBUG
	freopen("/usr/local/lib/chaos/tlog", "a", stderr);
#endif DEBUG
	pid = getpid();
	if (pipe(tord) < 0) {
		ioctl(0, CHIOCREJECT, "can't open pipe to read process");
		return(1);
	}
	if (pipe(towr) < 0) {
		ioctl(0, CHIOCREJECT, "?can't open pipe to write process");
		close(tord[0]);
		close(towr[0]);
		return(1);
	}
	for (c = 'p'; c < 'w'; c++)
		for (i = 0; i < 16; i++) {
			sprintf(pty, "/dev/pty%c%x", c, i);
			if ((f = open(pty, 2)) >= 0)
				goto win;
		}
#ifdef DEBUG
	fprintf(stderr, "TELNET couldn't open a pty\n");
	fflush(stderr);
#endif DEBUG
	ioctl(0, CHIOCREJECT, "?all pty's in use");
	return(1);
win:
	ioctl(0, CHIOCACCEPT, 0);
#ifdef DEBUG
	fprintf(stderr, "TELNET opened on %s\n", pty);
	fflush(stderr);
#endif DEBUG
	conn = dup(0);
	close(0);
	close(1);
	dup(f);
	dup(f);
	close(f);
	ioctl(conn, CHIOCSMODE, CHRECORD);
	read(conn, &junk, sizeof junk);
	setbuf(stdin, buf);
	isopen = 1;
	signal(SIGEMT, ipc);
	fk = fork();
	if (fk == 0) {
		fin = tord[0];
		fout = towr[1];
		close(tord[1]);
		close(towr[0]);
		wr_ipc(IPC_DUMMY);
		rd();
#ifdef DEBUG
		fprintf(stderr, "Connection closed by foreign host\r\n");
#endif DEBUG
		wr_ipc(IPC_CLOSED);
		kill(pid, SIGEMT);
		exit(3);
	}
	fin = towr[0];
	fout = tord[1];
	close(towr[1]);
	close(tord[0]);
	ipc();	/* reader always sends at least one */
	mode(3);
	wr();
	call(bye, "bye", 0);
	return(0);
}

bye(argc, argv)
int argc;
char *argv[];
{
	register int c;
	if (argc <= 0) {	/* give help */
		printf("Disconnect from the current site.\n");
		return(0);
	}
	mode(0);
	if (conn >= 0) {
		ioctl(conn, CHIOCOWAIT, 1);
		if (fk) {
			kill(fk, SIGKILL);
			wait((int *)NULL);
		}
		close(conn);
		if (fin > 0) {
			close(fin);
			close(fout);
		}
		conn = -1;
		fin = -1;
		fout = -1;
		if (isopen)
			printf("Connection closed\n");
		isopen = 0;
		fk = 0;
	}
	return(0);
}

/*
 * call routine with argc, argv set from args (terminated by 0).
 * VARARGS1
 */
call(routine, args)
int (*routine)();
int args;
{
	register int *argp;
	register int argc;
	for (argc = 0, argp = &args; *argp++ != 0; argc++);
	return((*routine)(argc,&args));
}

/*
 * read from the IPC pipe
 */
rd_ipc()
{
	long int waiting;
	char c;
	if (ioctl(fin, FIONREAD, &waiting) < 0
	 || waiting <= 0
	 || read(fin, &c, 1) != 1)
		return(EOF);
	return(c&0377);
}

/*
 * write to the IPC pipe
 */
wr_ipc(c)
char c;
{
	write(fout, &c, 1);
}

/*
 * handle IPC via the chaos connection between processes
 */
ipc()
{
	register int c;
	signal(SIGEMT, 1);
	while ((c = rd_ipc()) != EOF) {
		switch(c) {
		case IPC_MODE:
			mode(rd_ipc());
			break;
	
		case IPC_CLOSED:
			isopen = 0;
			break;
	
		case IPC_GA:
			goahead = rd_ipc();
			break;
	
		case IPC_DUMMY:
			break;

		case IPC_XMIT_BIN:
			myopts[TN_TRANSMIT_BINARY] = rd_ipc();
			break;
		}
	}
	signal(SIGEMT, ipc);
}

mode(f)
register int f;
{
	register int old;
	struct sgttyb stbuf;
	struct tchars tchars;
	struct ltchars ltchars;
	if (fk == 0 && fout >= 0) {	/* child process */
		wr_ipc(IPC_MODE);
		wr_ipc(f);
		kill(pid, SIGEMT);
		return;
	}
	ioctl(fileno(stdin), TIOCGETP, &stbuf);
	ioctl(fileno(stdin), TIOCGETC, &tchars);
	ioctl(fileno(stdin), TIOCGLTC, &ltchars);
	t_erase = stbuf.sg_erase;
	t_kill = stbuf.sg_kill;
	t_intrc = tchars.t_intrc;
	t_flushc = ltchars.t_flushc;
	old = echomode;
	echomode = f;
	switch (f) {
	case 0:
		stbuf.sg_flags &= ~RAW;
		stbuf.sg_flags |= ECHO|CRMOD;
		break;

	case 1:
		stbuf.sg_flags |= RAW;
		stbuf.sg_flags &= ~(ECHO|CRMOD);
		break;

	case 2:
		stbuf.sg_flags &= ~RAW;
		stbuf.sg_flags &= ~(ECHO|CRMOD);
		break;

	case 3:
		stbuf.sg_flags |= RAW;
		stbuf.sg_flags |= ECHO|CRMOD;
	}
	ioctl(fileno(stdin), TIOCSETN, &stbuf);
	return(old);
}
char wrlastc;	/* Last character written to net */
/*
 * flush the packet on alarm timers
 */
timer()
{
	signal(SIGALRM, timer);
	if (wrlastc == '\r')
		wrconn(wrlastc = 0);
	chflush(DATOP);
}

/*
 * read from the user's tty and write to the network connection
 */

wr()
{
	register int c;
	long int waiting;
	struct chstatus foo;
	extern int errno;
	opt(WILL, TN_ECHO);
	opt(DO, TN_SUPPRESS_GO_AHEAD);
	opt(WILL, TN_SUPPRESS_GO_AHEAD);
	chflush(DATOP);
	sleep(2);	/* Cheap delay for negotiations to happen */
	wrlastc = 0;
	signal(SIGALRM, timer);
loop:
	while (alarm(1), errno = 0, (c = getchar()) != EOF) {
		alarm(0);	/* disable timer after the read */
		if (!myopts[TN_TRANSMIT_BINARY])
			c &= 0177;	/* strip the parity bit */
		if (wrlastc == '\r' && c != '\n')
			wrconn(0);
		wrconn(c);
		wrlastc = c;
		/****************************************
		 * DEPENDS ON UNIX STDIO IMPLEMENTATION *
		 ****************************************/
		if (stdin->_cnt <= 0
		 && ioctl(fileno(stdin), FIONREAD, &waiting) >= 0
		 && waiting == 0) {
		 	int n = ioctl(conn, CHIOCGSTAT, &foo);
#ifdef DEBUG
/*			fprintf(stderr, "ioctl = %d room = %d\n",  */
/*				n, foo.st_oroom);		   */
/*			fflush(stderr);				   */
#endif
			if ((n >= 0) && (foo.st_state != CSOPEN) && (foo.st_plength))
				return(0);	/* connection dead and nothing to send */
			if ( n >= 0 && foo.st_oroom > 0)
				chflush(DATOP);
		}
	}
	if (errno == EINTR && isopen) {
		clearerr(stdin);
		goto loop;
	}
	return(0);
}

rd()
{
	register int c, lastc;
	extern int errno;

	lastc = 0;
	while (1) {
		if (emptyconn()) {
			fflush(stdout);
			if (goahead) {
				wrconn(IAC);
				wrconn(GA);
				chflush(DATOP);
			}
		}
		errno = 0;
		if ((c = rdconn()) == EOF) {
			if (errno == EINTR)
				continue;
			break;
		}
		alarm(0);
		if (c == IAC)
			option();
		else {
			if (lastc != '\r' || (c != '\n' && c != 0))
				putchar(c);
			lastc = c;
		}
	}
}

option()
{
	register int code;
	register int c;
	struct sgttyb stbuf;
	struct tchars tchars;
	struct ltchars ltchars;
	if ((c = rdconn()) == EOF)
		return;
	/* update local characters */
	ioctl(fileno(stdout), TIOCGETP, &stbuf);
	ioctl(fileno(stdout), TIOCGETC, &tchars);
	ioctl(fileno(stdout), TIOCGLTC, &ltchars);
	t_erase = stbuf.sg_erase;
	t_kill = stbuf.sg_kill;
	t_intrc = tchars.t_intrc;
	t_flushc = ltchars.t_flushc;
	switch(code = c) {
	case WILL:
	case WONT:
	case DO:
	case DONT:
		c = rdconn();
		switch(code) {
		case WILL:
			if (hisopts[c])	/* already active */
				break;
			if (c == TN_SUPPRESS_GO_AHEAD) {
				hisopts[c] = 1;
				opt(DO, c);
			} else if (c == TN_TRANSMIT_BINARY) {
				hisopts[c] = 1;
				opt(DO, c);
			} else if (c == TN_TIMING_MARK) {
				opt(DONT, c);
			} else {
				opt(DONT, c);
			}
			break;

		case WONT:
			if (!hisopts[c])	/* already inactive */
				break;
			if (c == TN_SUPPRESS_GO_AHEAD) {
				opt(DONT, c);
				hisopts[c] = 0;
			} else if (c == TN_TRANSMIT_BINARY) {
				opt(DONT, c);
				hisopts[c] = 0;
			} else {
				opt(DONT, c);
			}
			break;

		case DO:
			if (myopts[c])	/* already active */
				break;
			if (c == TN_ECHO) {
				mode(3);
				opt(WILL, c);
				myopts[c] = 1;
				hisopts[c] = 0;
			} else if (c == TN_TIMING_MARK) {
				opt(WONT, c);
			} else if (c == TN_SUPPRESS_GO_AHEAD) {
				wr_ipc(IPC_GA);
				wr_ipc(0);
				kill(pid, SIGEMT);
				opt(WILL, c);
				myopts[c] = 1;
				goahead = 0;
			} else if (c == TN_TRANSMIT_BINARY) {
				opt(WILL, c);
				myopts[c] = 1;
				wr_ipc(IPC_XMIT_BIN);
				wr_ipc(1);
				kill(pid, SIGEMT);
			} else {
				opt(WONT, c);
			}
			break;

		case DONT:
			if (!myopts[c]) /* already inactive */
				break;
			myopts[c] = 0;
			if (c == TN_ECHO) {
				mode(1);
				hisopts[c] = 1;
				opt(WONT, c);
			} else if (c == TN_TIMING_MARK) {
				opt(WONT, c);
			} else if (c == TN_SUPPRESS_GO_AHEAD) {
				wr_ipc(IPC_GA);
				wr_ipc(1);
				kill(pid, SIGEMT);
				opt(WONT, c);
				goahead = 1;
			} else if (c == TN_TRANSMIT_BINARY) {
				wr_ipc(IPC_XMIT_BIN);
				wr_ipc(0);
				opt(WONT, c);
				kill(pid, SIGEMT);
			} else {
				opt(WONT, c);
			}
			break;
		}
		chflush(DATOP);
		break;

	case DMARK:
		fflush(stdout);	/* write the output */
		ioctl(fileno(stdout), TIOCFLUSH, 0);
		break;

	case IP:
		putchar(t_intrc);
		break;

	case EC:
		putchar(t_erase);
		break;

	case EL:
		putchar(t_kill);
		break;

	case AO:
		wrconn(IAC);
		wrconn(DMARK);
		chflush(DATOP);
		putchar(t_flushc);
		break;

	case NOP:
	case GA:
		break;

#ifdef DEBUG
	default:
		fprintf(stderr, "?Received IAC %d\r\n", code);
#endif DEBUG
	}
}

/*
 * send an option
 */
opt(option, value)
int option, value;
{
	wrconn(IAC);
	wrconn(option);
	wrconn(value);
	chflush(DATOP);
}

static struct chpacket pkt;
static char *ptr = pkt.cp_data;
static cnt = 0;

/*
 * read a character from the network connection
 */
rdconn()
{
	char c;
	if (cnt-- > 0)
		return(*ptr++&0377);
	if ((cnt = read(conn, &pkt, sizeof(pkt))) <= 0)
		return(EOF);
	switch (pkt.cp_op&0377) {
	case DATOP:
		break;

	case CLSOP:
	case EOFOP:
	case LOSOP:
		return(EOF);

	default:
#ifdef DEBUG
		fprintf(stderr, "?packet op = %o len = %d\r\n",
			pkt.cp_op&0377, cnt);
#endif DEBUG
		break;
	}		
	cnt--;	/* skip past the opcode */
	ptr = pkt.cp_data+1;
	cnt--;
	return(pkt.cp_data[0]&0377);
}

/*
 * return true if nothing can be read from the connection
 */
emptyconn()
{
	long int waiting;
	if (cnt <= 0
	 || ioctl(conn, FIONREAD, &waiting) < 0
	 || waiting == 0)
		return(1);
	return(0);
}

/*
 * save up data to be sent
 */

struct chpacket opkt;	/* current output packet */
char *optr = opkt.cp_data;
int ocnt = sizeof opkt.cp_data;

/*
 * write a character to the connection
 */
wrconn(c)
int c;
{
	*optr++ = c;
	if (--ocnt <= 0)
		chflush(DATOP);
}

/*
 * flush the connection in a packet with the specified opcode
 */
chflush(op)
int op;
{
	if (optr==opkt.cp_data)
		return;
	opkt.cp_op = op;
	write(conn, &opkt, optr-opkt.cp_data+1);
	optr = opkt.cp_data;
	ocnt = sizeof(opkt.cp_data);
}

/*
 * send a synch code
 */
synch()
{
	chflush(DATOP);
	wrconn(IAC);
	wrconn(DMARK);
	chflush(0201);
}
