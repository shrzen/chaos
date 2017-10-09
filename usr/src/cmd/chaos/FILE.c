#include <chaos/user.h>
#include <stdio.h>
#include <ctype.h>
#include <time.h>
#include <sys/param.h>	/* for BSIZE */
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <errno.h>
#include <pwd.h>
#include <setjmp.h>
#include <signal.h>
#ifdef SYSLOG
#include <syslog.h>
#else
#define LOG_INFO	0
#define LOG_ERR		1
/* VARARGS 2 */
syslog(n,a,b,c,d,e) char *a;{if (n) fprintf(stderr,a,b,c,d,e);}
#endif
#ifdef pdp11
#ifndef lint
#ifndef void
#define void int
#endif
#endif
#endif
/*
 * Chaosnet file protocol server.
 */
#define	BIT(n)	(1<<(n))
/*
 * Structure describing each transfer in progress.
 * Essentially each OPEN or DIRECTORY command creates a transfer
 * process, which runs (virtually) asynchronously with respect to the
 * control/command main process.  Certain transactions/commands
 * are performed synchronously with the control/main process, like
 * OPEN-PROBES, completion, etc., while others relate to the action
 * of a transfer process (e.g. SET-BYTE-POINTER).
 * Until the Berkeley select mechanism is done, we must implement transfers
 * as sub-processes.
 * The #ifdef SELECT code is for when that time comes (untested, incomplete).
 */
struct xfer		{
	struct xfer		*x_next;	/* Next in global list */
	struct xfer		*x_runq;	/* Next in runnable list */
	char			x_state;	/* Process state */
	struct file_handle	*x_fh;		/* File handle of process */
	struct transaction	*x_work;	/* Queued transactions */
	int			x_options;	/* OPEN options flags */
	int			x_flags;	/* Randome state bits */
	int			x_fd;		/* UNIX file descriptor */
	char			*x_realname;	/* filename */
	int			x_left;		/* Bytes in input buffer */
	int			x_room;		/* Room in output buffer */
	char			*x_bptr;	/* Disk buffer pointer */
	char			*x_pptr;	/* Packet buffer pointer */
	char			x_bbuf[BSIZE];	/* Disk buffer */
	struct chpacket		x_pkt;		/* Packet buffer */
#define	x_op			x_pkt.cp_op	/* Packet buffer opcode */
#define x_pbuf			x_pkt.cp_data	/* Packet data buffer */
	char			**x_glob;	/* Files for DIRECTORY */
	char			**x_gptr;	/* Ptr into x_glob vector */
#ifndef SELECT
	int			x_pfd;		/* Subprocess pipe file */
	int			x_pid;		/* Subprocess pid */
#endif
} *xfers;

#define XNULL	((struct xfer *)0)
/*
 * Values for x_state.
 */
#define	X_BROKEN	0	/* An error on the data connection occurred */
#define	X_PROCESS	1	/* Work is in process */
#define X_HUNG		2	/* Hung, awaiting further instructions */
#define X_DONE		3	/* Successful completion has occurred */
#define	X_ABORT		4	/* Command was aborted */
#define X_ERROR		5	/* Hung after error */
/*
 * Values for x_flags
 */
#define X_EOF		BIT(1)	/* EOF has been read from network */
#define X_QUOTED	BIT(2)	/* Used in character set translation */
#define X_SYNOP		BIT(3)	/* Synchronous mark already read */
/*
 * Structure describing each command.
 * Created by the command parser (getcmd) and extant while the command is
 * in progress until it is done and responded to.
 */
struct transaction	{
	struct transaction	*t_next;	/* For queuing work on xfers*/
	char			*t_tid;		/* Id. for this transaction */
	struct file_handle	*t_fh;		/* File handle to use */
	struct command		*t_command;	/* Command to perform */
	struct cmdargs		*t_args;	/* Args for this command */
};
#define	TNULL	((struct transaction *) 0)

/*
 * Structure for each command.
 * Used by the parser for argument syntax and holds the actual function
 * which performs the work.
 */
struct command		{
	char			*c_name;	/* Command name */
	int			(*c_func)();	/* Function to call */
	int			c_flags;	/* Various bits. See below */
	char			*c_syntax;	/* Syntax description */
};
#define	CNULL	((struct command *)0)
/*
 * Bit values for c_flags
 */
#define	C_FHMUST	BIT(0)	/* File handle must be present */
#define	C_FHCANT	BIT(1)	/* File handle can't be present */
#define C_FHINPUT	BIT(2)	/* File handle must be for input */
#define C_FHOUTPUT	BIT(3)	/* File handle must be for output */
#define C_XFER		BIT(4)	/* Command should be queued on a transfer */
#define C_NOLOG		BIT(5)	/* Command permitted when not logged in */
/*
 * Structure of arguments to commands.
 */
#define MAXSTRINGS	3	/* Maximum # of string arguments in any cmd */
#define MAXNUMS		2	/* "		numeric " */
struct cmdargs	{
	int			a_options;		/* Option bits */
	char			*a_string[MAXSTRINGS];	/* Random strings */
	long			a_num[MAXNUMS];		/* Random numbers */
	struct plist		*a_plist;		/* Property list */
};
#define ANULL	((struct cmdargs *)0)
struct plist	{
	struct plist	*p_next;
	char		*p_name;
	char		*p_value;
};
#define PNULL	((struct plist *)0)

/*
 * Structure for each "file handle", essentially one side of a
 * bidirectional chaos "data connection".
 */
struct file_handle	{
	struct file_handle	*f_next;	/* Next on global list */
	char			*f_name;	/* Identifier from user end */
	int			f_type;		/* See below */
	int			f_fd;		/* UNIX file descriptor */
	struct xfer		*f_xfer;	/* Active xfer on this fh */
	struct file_handle	*f_other;	/* Other fh of in/out pair */
} *file_handles;
#define	FNULL	((struct file_handle *)0)
/*
 * Values for f_type
 */
#define	F_INPUT		0	/* Input side of connection */
#define F_OUTPUT	1	/* Output size of connection */

/*
 * Protocol errors
 */
struct file_error	{
	char		e_type;		/* Type of error - see below */
	char		*e_code;	/* Standard three letter code */
	char		*e_string;	/* Error message string */
};
/*
 * Values of e_type
 */
#define E_COMMAND	'C'		/* Command error */
#define	E_FATAL		'F'		/* Fatal transfer error */
#define	E_RECOVERABLE	'R'		/* Recoverable transfer error */

/*
 * Command options.
 */
struct option	{
	char	*o_name;	/* Name of option */
	char	o_type;		/* Type of value */
	int	o_value;	/* Value of option */
	int	o_cant;		/* Bit values of mutually exclusive options*/
};
/*
 * Values for o_type
 */
#define	O_BIT	0	/* Value is bit to turn on in a_options */
#define O_NUMBER	1	/* Value is (SP, NUM) following name */
/*
 * Values for o_value
 */
#define	O_READ		BIT(0)
#define	O_WRITE		BIT(1)
#define	O_PROBE		BIT(2)
#define	O_CHARACTER	BIT(3)
#define	O_BINARY	BIT(4)
#define	O_RAW		BIT(5)
#define	O_SUPER		BIT(6)
#define	O_TEMPORARY	BIT(7)
#define	O_DELETED	BIT(8)
#define	O_OLD		BIT(9)
#define	O_NEWOK		BIT(10)
#define O_DEFAULT	BIT(11)
#define O_DIRECTORY	BIT(12)	/* Used internally - not a keyword option */
#define O_FAST		BIT(13)	/* Fast directory list - no properties */
#define O_PRESERVE	BIT(14)	/* Preserve reference dates - not implemented */
/*
 * Structure allocation macros.
 */
#define	salloc(s)	(struct s *)malloc(sizeof(struct s))
#define sfree(sp)	free((char *)sp)
#define NOSTR		((char *)0)
/*
 * Return values from dowork.
 */
#define X_FLUSH		0	/* Flush the transfer, its all over */
#define X_CONTINUE	1	/* Everything's ok, just keep going */
#define X_HANG		2	/* I need more commands to continue */
/*
 * Constants of the protocol
 */
#define	TIDLEN		5		/* Significant length of tid's */
#define FHLEN		5		/* Significant length of fh's */
#define SYNOP		0201		/* Opcode for synchronous marks */
#define ASYNOP		0202		/* Opcode for asynchronous marks */
#define NIL		"NIL"		/* False value in binary options */
#define T		"T"		/* True value in binary options */
#define QFASL1		0143150		/* First word of "QFASL" in SIXBIT */
#define QFASL2		071660		/* Second word of "QFASL" */
#ifdef pdp11
#define QFASL		((((long)QFASL1)<<16)+(QFASL2))
#else
#define QFASL		((((long)QFASL2)<<16)+(QFASL1))
#endif

#ifndef SELECT
/*
 * The structure in which a command is sent to a transfer process.
 */
struct pmesg {
	char		pm_tid[TIDLEN + 1];	/* TIDLEN chars of t->t_tid */
	struct command	*pm_cmd;		/* Actual t_command */
	char		pm_args;		/* 0 = no args, !0 = args */
	long		pm_n;			/* copy of t_args->a_num[0] */
};
int ctlpipe[2];					/* Pipe - xfer proc to ctl */
						/* Just PID's are written */
jmp_buf closejmp;
int closesig();
int nprocdone;					/* Number of processes done */
#endif
/*
 * Definitions of errors
 */
struct file_error	errors[] = {
/*		type		code	message */
#define CCC	0
{		E_COMMAND,	"CCC",	"Channel cannot continue."},
#define CNO	1
{		E_COMMAND,	"CNO",	"Channel not open."},
#define CRF	2
{		E_COMMAND,	"CRF",	"Cannot rename file."},
#define CSP	3
{		E_FATAL,	"CSP",	"Cannot set pointer."},
#define	FNF	4
{		E_COMMAND,	"FNF",	"File not found."},
#define	IBS	5
{		E_COMMAND,	"IBS",	"Illegal byte size."},
#define	IDO	6
{		E_FATAL,	"IDO",	"Illegal data opcode."},
#define	IFH	7
{		E_COMMAND,	"IFH",	"Illegal file handle."},
#define	IOC	8
{		E_RECOVERABLE,	"IOC",	"Input/Output condition."},
#define	IPO	9
{		E_FATAL,	"IPO",	"Illegal packet opcode."},
#define	IRF	10
{		E_COMMAND,	"IRF",	"Illegal request format."},
#define	ISC	11
{		E_COMMAND,	"ISC",	"Illegal SET-BYTE-SIZE channel."},
#define	NCN	12
{		E_COMMAND,	"NCN",	"Null command name."},
#define	NER	13
{		E_COMMAND,	"NER",	"Not enough resources."},
#define	NLI	14
{		E_COMMAND,	"NLI",	"Not logged in."},
#define	TMI	15
{		E_COMMAND,	"TMI",	"Too much information."},
#define	UFH	16
{		E_COMMAND,	"UFH",	"Unknown file handle."},
#define	UKC	17
{		E_COMMAND,	"UKC",	"Unknown command."},
#define	UNK	18
{		E_COMMAND,	"UNK",	"User not known."},
#define	UOO	19
{		E_COMMAND,	"UOO",	"Unknown OPEN option."},
#define RAN	20
{		E_COMMAND,	"IOC",	"Other errors of our own."},
#define NSD	21
{		E_COMMAND,	"NSD",	"No such directory."},
};

/*
 * Definition of options
 */
struct option options[] = {
/*	o_name		o_type		o_value		o_cant	 */
{	"READ",		O_BIT,		O_READ,		O_WRITE|O_PROBE	},
{	"WRITE",	O_BIT,		O_WRITE,	O_READ|O_PROBE	},
{	"PROBE",	O_BIT,		O_PROBE,	O_READ|O_WRITE	},
{	"CHARACTER",	O_BIT,		O_CHARACTER,	O_BINARY|O_DEFAULT},
{	"BINARY",	O_BIT,		O_BINARY,	O_CHARACTER|O_DEFAULT},
{	"BYTE-SIZE",	O_NUMBER,	0,		O_CHARACTER	},
{	"RAW",		O_BIT,		O_RAW,		O_BINARY|O_SUPER},
{	"SUPER-IMAGE",	O_BIT,		O_SUPER,	O_BINARY|O_RAW	},
{	"TEMPORARY",	O_BIT,		O_TEMPORARY,	0},
{	"DELETED",	O_BIT,		O_DELETED,	0},
{	"OLD",		O_BIT,		O_OLD,		O_NEWOK	},
{	"NEW_OK",	O_BIT,		O_NEWOK,	O_OLD	},
{	"DEFAULT",	O_BIT,		O_DEFAULT,	O_BINARY|O_CHARACTER},
{	"FAST",		O_BIT,		O_FAST,		0},
{	"PRESERVE-DATES",O_BIT,		O_PRESERVE,	0},
{	NOSTR,	},
};

/*
 * Syntax definitions values for syntax strings
 */
#define	SP	1	/* Must be CHSP character */
#define	NL	2	/* Must be CHNL character */
#define	STRING	3	/* All characters until following character */
#define OPTIONS	4	/* Zero or more (SP, WORD), WORD in options */
#define NUMBER	5	/* [-]digits, base ten */
#define OPEND	6	/* Optional end of command */
#define PNAME	7	/* Property name */
#define PVALUE	8	/* Property value */
#define REPEAT	9	/* Repeat from last OPEND */
#define END	10	/* End of command, must be at end of args */

char	dcsyn[] =	{ SP, STRING, SP, STRING, END };
char	nosyn[] =	{ END };
char	opensyn[] =	{ OPTIONS, NL, STRING, NL, END };
char	possyn[] =	{ SP, NUMBER, END };
char	delsyn[] =	{ OPEND, NL, STRING, NL, END };
char	rensyn[] =	{ NL, STRING, NL, OPEND, STRING, NL, END };
/*char	setsyn[] =	{ SP, NUMBER, SP, OPEND, NUMBER, END };*/
char	logsyn[] =	{ SP, OPEND, STRING, OPEND, SP, STRING, OPEND,
			  SP, STRING, END };
char	dirsyn[] =	{ OPTIONS, NL, STRING, NL, END };
char	compsyn[] =	{ OPTIONS, NL, STRING, NL, STRING, END };
char	changesyn[] =	{ STRING, NL, OPEND, PNAME, SP, PVALUE, NL, REPEAT };
char	crdirsyn[] =	{ NL, STRING, END };
/*
 * Command definitions
 */
int	dataconn(), undataconn(), fileopen(), fileclose(), filepos(),
	delete(), rename(), filecontinue(), setbytesize(), login(),
	directory(), complete(), changeprop(), crdir()
	;
struct command commands[] = {
/*	c_name			c_funct		c_flags		c_syntax */
{	"DATA-CONNECTION",	dataconn,	C_FHCANT|C_NOLOG,dcsyn	},
{	"UNDATA-CONNECTION",	undataconn,	C_FHMUST|C_NOLOG,nosyn	},
{	"OPEN",			fileopen,	0,		opensyn	},
{	"CLOSE",		fileclose,	C_FHMUST,	nosyn	},
{	"FILEPOS",		filepos,	C_XFER|C_FHINPUT,possyn	},
{	"DELETE",		delete,		0,		delsyn	},
{	"RENAME",		rename,		0,		rensyn	},
{	"CONTINUE",		filecontinue,	C_XFER|C_FHMUST,nosyn	},
/*{	"SET-BYTE-SIZE",	setbytesize,	C_XFER|C_FHINPUT,setsyn	},*/
{	"LOGIN",		login,		C_FHCANT|C_NOLOG,logsyn	},
{	"DIRECTORY",		directory,	C_FHINPUT,	dirsyn	},
{	"COMPLETE",		complete,	C_FHCANT,	compsyn	},
{	"CHANGE-PROPERTIES",	changeprop,	C_FHCANT,	changesyn },
{	"CREATE-DIRECTORY",	crdir,		C_FHCANT,	crdirsyn },
{	NOSTR	},
};
/*
 * Kludge for actually responding to the DIRECTORY command in the subtask.
 */
int dirstart();
struct command dircom = {
	"DIRECTORY",	dirstart,	0,		0
};
/*
 * Fatal internal error messages
 */
#define NOMEM		"Out of memory"
#define BADSYNTAX	"Bad syntax definition in program"
#define CTLWRITE	"Write error on control connection"
#define BADFHLIST	"Bad file_handle list"
#define FSTAT		"Fstat failed"

/*
 * Our own error messages for error responses,
 */
#define WRITEDIR	"Permission denied for modifying directory."
#define PERMDIR		"Permission denied for searching directory."
#define PERMFILE	"Permission denied on file."
#define PATHNOTDIR	"One of the pathname components is not a directory."
#define MISSDIR		"Directory doesn't exist."

/*
 * Globals
 */
short useraddr;			/* Net address of user */
struct chstatus chst;		/* Status buffer for connections */
struct stat sbuf;		/* Used so often, made global */
char *errstring;		/* Error message if non-standard */
char *home;			/* Home directory, after logged in */
char *cwd;			/* Current working directory, if one */
int protocol;			/* Which number argument after FILE in RFC */
int mypid;			/* Pid of controlling process */
extern int errno;		/* System call error code */
extern char *sys_errlist[];	/* System error messages */

/*
 * Non-integer return types from functions
 */
char *savestr(), *malloc(), *strcpy(), *sprintf(), *rindex(), *strcat();
char *fullname(), *crypt();
long lseek(), fseek();
struct passwd *getpwnam(), *getpwuid();
char *parsepath(), *mv();
struct tm *localtime();
struct transaction *getwork();
char **glob();

/*
 * This server gets called from the general RFC server, who gives us control
 * with standard input and standard output set to the control connection
 * and with the connection open (already accepted)
 */
main(argc, argv)
int argc;
char **argv;
{
	register struct transaction *t;
	struct transaction *getwork();

#ifndef SYSLOG
	close(2);
	(void)open("/usr/src/local/cmd/chaos/FILElog", 1);
	lseek(2, 0L, 2);
#endif

	if (argc > 1)
		protocol = atoi(argv[1]);
	if (protocol > 1) {
		syslog(LOG_ERR, "FILE: protocol I can't handle: %d\n",
			protocol);
		ioctl(0, CHIOCREJECT, "Unknown FILE protocol version");
		exit(1);
	} else
		ioctl(0, CHIOCACCEPT, 0);

	mypid = getpid();
	ioctl(0, CHIOCSMODE, CHRECORD);
	ioctl(0, CHIOCGSTAT, &chst);
	useraddr = chst.st_fhost;

	while (t = getwork())
		if (t->t_command->c_flags & C_XFER)
			xcommand(t);
		else
			(*t->t_command->c_func)(t);
	syslog(LOG_INFO, "FILE: exiting normally\n");
	finish();
}
/*
 * Getwork - read a command from the control connection (standard input),
 *	parse it, including all arguments (as much as possible),
 *	and do any possible error checking.  Any errors cause rejection
 *	here, and such commands are not returned.
 */
 struct transaction *
getwork()
{
	register struct transaction *t;
	register struct command *c;
	register char *cp;
	register int length;
	char *fhname, *cname, save;
	int errcode;
	struct chpacket p;
	
	for (;;) {
		if ((t = salloc(transaction)) == TNULL)
			fatal(NOMEM);

		t->t_tid = NOSTR;
		t->t_fh = FNULL;
		t->t_command = CNULL;
		t->t_args = ANULL;
		fhname = "";
		errcode = IRF;

#ifndef SELECT
		xcheck();
#endif
		if ((length = read(0, (char *)&p, sizeof(p))) <= 0) {
			syslog(LOG_ERR, "FILE: Ctl connection broken(%d,%d)\n",
				length, errno);
			tfree(t);
			return (TNULL);
		}
#ifndef SELECT
		xcheck();
#endif

		((char *)&p)[length] = '\0';
		syslog(LOG_INFO, "FILE: pkt(%d):%.*s\n", p.cp_op&0377, length-1,
			p.cp_data);
		switch(p.cp_op & 0377) {
		case EOFOP:
			tfree(t);
			return TNULL;
		case CLSOP:
		case LOSOP:
			syslog(LOG_INFO, "FILE: Close pkt: '%s'\n", p.cp_data);
			tfree(t);
			return TNULL;
		case RFCOP:
			continue;
		case DATOP:
			break;
		default:
			syslog(LOG_ERR, "FILE: Bad op: 0%o\n", p.cp_op&0377);
			tfree(t);
			return TNULL;
		}
		for (cp = p.cp_data; *cp != CHSP; )
			if (*cp++ == '\0')
				goto cmderr;
		*cp = '\0';
		t->t_tid = savestr(p.cp_data);
		if (*++cp != CHSP) {
			struct file_handle *f;

			for (fhname = cp; *++cp != CHSP; )
				if (*cp == '\0')
					goto cmderr;
			*cp = '\0';
			for (f = file_handles; f; f = f->f_next)
				if (strncmp(fhname, f->f_name, FHLEN) == 0)
					break;
			if (f == FNULL) {
				errcode = UFH;
				goto cmderr;
			}
			t->t_fh = f;
		}
		for (cname = ++cp; *cp != '\0' && *cp != CHSP && *cp != CHNL;)
			cp++;
		save = *cp;
		*cp = '\0';
		if (*cname == '\0') {
			errcode = NCN;
			goto cmderr;
		}
		for (c = commands; c->c_name; c++)
			if (strcmp(cname, c->c_name) == 0)
				break;
		if (c == CNULL) {
			errcode = UKC;
			goto cmderr;
		}
		if (home == NOSTR && (c->c_flags & C_NOLOG) == 0) {
			errcode = NLI;
			goto cmderr;
		}
		t->t_command = c;
		*cp = save;
		if (t->t_fh == FNULL) {
			if (c->c_flags & (C_FHMUST|C_FHINPUT|C_FHOUTPUT))
				goto cmderr;
		} else if (c->c_flags & C_FHCANT)
			goto cmderr;
		else if ((c->c_flags & C_FHINPUT &&
			  t->t_fh->f_type != F_INPUT) ||
			 (c->c_flags & C_FHOUTPUT &&
			  t->t_fh->f_type != F_OUTPUT)) {
			errcode = IFH;
			goto cmderr;
		}
		if ((errcode = parseargs(cp, c, &t->t_args)) == 0)
			break;
	cmderr:
		error(t, fhname, errcode);
	}
	return t;
}
/*
 * Parse the argument part of the command, returning an error code if
 * an error was found.
 */
parseargs(args, c, argp)
char *args;
struct command *c;
struct cmdargs **argp;
{
	register char *syntax;
	register struct cmdargs *a;
	struct plist *p;
	struct option *o;
	int nnums, nstrings;
	long n;
	char *string();

	*argp = ANULL;
	if (c->c_syntax[0] == END)
		if (args[0] == '\0')
			return 0;
		else
			return IRF;
	if ((a = salloc(cmdargs)) == ANULL)
		fatal(NOMEM);
	ainit(a);
	nstrings = nnums = 0;
	for (syntax = c->c_syntax; *syntax != END; ) {
		switch (*syntax++) {
		case SP:
			if (*args++ != CHSP)
				goto synerr;
			continue;
		case NL:
			if (*args++ != CHNL)
				goto synerr;
			continue;
		case OPEND:
			if (*args == '\0')
				break;
			continue;
		case REPEAT:
			while (*--syntax != OPEND)
				;
			continue;
		case NUMBER:
			if (!isdigit(*args))
				goto synerr;
			n = 0;
			while (isdigit(*args)) {
				n *= 10;
				n += *args++ - '0';
			}
			a->a_num[nnums++] = n;
			continue;
		case STRING:
			a->a_string[nstrings++] = string(&args, syntax);
			continue;
		case PNAME:
			if ((p = a->a_plist) != PNULL && p->p_value == NOSTR)
				goto synerr;
			if ((p = salloc(plist)) == PNULL)
				fatal(NOMEM);
			p->p_name = string(&args, syntax);
			p->p_next = a->a_plist;
			a->a_plist = p;
			p->p_value = NOSTR;
			continue;
		case PVALUE:
			if ((p = a->a_plist) == PNULL || p->p_value != NOSTR)
				goto synerr;
			p->p_value = string(&args, syntax);
			continue;
		case OPTIONS:
			while (*args == CHSP) {
				char *oname;

				args++;
				oname = string(&args, "");
				if (*oname == '\0')
					continue;
				for (o = options; o->o_name; o++)
					if (strcmp(oname, o->o_name) == 0)
						break;
				free(oname);
				if (o->o_name == NOSTR) {
					syslog(LOG_ERR, "FILE: UOO:'%s'\n",
						oname);
					return (UOO);
				}
				switch (o->o_type) {
				case O_BIT:
					a->a_options |= o->o_value;
					break;
				case O_NUMBER:
					if (*args++ != CHSP ||
					    !isdigit(*args))
						goto synerr;
					n = 0;
					while (isdigit(*args)) {
						n *= 10;
						n += *args++ - '0';
					}
					a->a_num[nnums++] = n;
					break;
				}
			}
			for (o = options; o->o_name; o++)
				if (o->o_type == O_BIT &&
				    (o->o_value & a->a_options) != 0 &&
				    (o->o_cant & a->a_options))
					goto synerr;
			continue;
		default:
			fatal("Bad token type in syntax");
		}
		break;
	}
	*argp = a;
	return 0;
synerr:
	afree(a);
	return IRF;
}
/*
 * Return the saved string starting at args and ending at the
 * point indicated by *term.  Return the saved string, and update the given
 * args pointer to point at the terminating character.
 * If term is null, terminate the string on CHSP, CHNL, or null.
 */
 char *
string(args, term)
register char **args;
char *term;
{
	register char *cp;
	register char *s;
	char delim, save;

	if (*term == OPEND)
		term++;

	switch (*term) {
	case SP:
		delim = CHSP;
		break;
	case NL:
		delim = CHNL;
		break;
	case END:
		delim = '\0';
	case 0:
		delim = '\0';
		break;
	default:
		fatal("Bad delimiter for string");
	}
	for (cp = *args; *cp != delim; cp++)
		if (*cp == '\0' || *term == 0 && (*cp == CHSP || *cp == CHNL))
			break;
	save = *cp;
	*cp = '\0';
	s = savestr(*args);
	*cp = save;
	*args = cp;
	return s;
}

/*
 * Free storage specifically associated with a transaction (not file handles)
 */
tfree(t)
register struct transaction *t;
{
	if (t->t_tid)
		free(t->t_tid);
	if (t->t_args)
		afree(t->t_args);
	sfree(t);
}
/*
 * Free storage in an argumet structure
 */
afree(a)
register struct cmdargs *a;
{
	register char **ap, i;

	for (ap = a->a_string, i = 0; i < MAXSTRINGS; ap++, i++)
		if (*ap)
			free(*ap);
	pfree(a->a_plist);
}
/*
 * Free storage in a plist.
 */
pfree(p)
register struct plist *p;
{
	register struct plist *np;

	while (p) {
		if (p->p_name)
			free(p->p_name);
		if (p->p_value)
			free(p->p_value);
		np = p->p_next;
		sfree(p);
		p = np;
	}
}

/*
 * Make a static copy of the given string.
 */
 char *
savestr(s)
char *s;
{
	register char *p;

	if ((p = malloc((unsigned)(strlen(s) + 1))) == NOSTR)
		fatal(NOMEM);
	(void)strcpy(p, s);
	return p;
}

/*
 * Initialize an argument struct
 */
ainit(a)
register struct cmdargs *a;
{
	register int i;

	a->a_options = 0;
	for (i = 0; i < MAXSTRINGS; i++)
		a->a_string[i] = NOSTR;
	for (i = 0; i < MAXNUMS; i++)
		a->a_num[i] = 0;
	a->a_plist = PNULL;
}
/*
 * Blow me away completely. I am losing.
 */
/* VARARGS 1*/
fatal(s, a1, a2, a3)
char *s;
{
	syslog(LOG_ERR, "Fatal error in chaos file server: ");
	syslog(LOG_ERR, s, a1, a2, a3);
	syslog(LOG_ERR, "\n");
	if (getpid() != mypid)
		kill(mypid, SIGTERM);

        exit(1);
}
/*
 * Respond to the given transaction, including the given results string.
 * If the result string is non-null a space is prepended
 */
respond(t, results)
register struct transaction *t;
char *results;
{
	register int len;
	struct chpacket p;

	p.cp_op = DATOP;
	(void)sprintf(p.cp_data, "%s %s %s%s%s", t->t_tid,
		t->t_fh != FNULL ? t->t_fh->f_name : "",
		t->t_command->c_name, results != NOSTR ? " " : "",
		results != NOSTR ? results : "");
	len = 1 + strlen(p.cp_data);
	if (write(1, (char *)&p, len) != len)
		fatal(CTLWRITE);
	tfree(t);
}
/*
 * The transaction found an error, inform the other end.
 * If errsting has been set, use it as the message, otherwise use the
 * standard one.
 */
error(t, fh, code)
struct transaction *t;
char *fh;
{
	struct chpacket p;
	register struct file_error *e = &errors[code];
	register int len;

	p.cp_op = DATOP;
	(void)sprintf(p.cp_data, "%s %s ERROR %s %c %s", t->t_tid, fh,
		e->e_code, e->e_type, errstring ? errstring : e->e_string);
	errstring = NOSTR;
	len = 1 + strlen(p.cp_data);
	if (write(1, (char *)&p, len) != len)
		fatal(CTLWRITE);
	tfree(t);
}
/*
 * Send a synchronous mark on the given file handle.
 * It better be for output!
 */
syncmark(f)
register struct file_handle *f;
{
	char op;

	if (f->f_type != F_INPUT)
		fatal("Syncmark");
	op = SYNOP;
	if (write(f->f_fd, &op, 1) != 1)
		return -1;
	syslog(LOG_INFO, "FILE: wrote syncmark to net\n");
	return 0;
}

/*
 * Create a new data connection.  Output file handle (second string argument
 * is the contact name the other end is listening for.  We must send an RFC.
 * The waiting for the new connection is done synchronously, and thus prevents
 * tha handling of any other commands for a bit.  It should, if others already
 * exist, create a transfer task, just for the purpose of waiting. Yick.
 */
dataconn(t)
register struct transaction *t;
{
	register struct file_handle *ifh, *ofh;
	char *ifhname, *ofhname;
	char rfc[CHMAXDATA];
	int fd;

	ifhname = t->t_args->a_string[0];
	ofhname = t->t_args->a_string[1];

	/*
	 * Make sure that our "new" file handle names are really new.
	 */
	for (ifh = file_handles; ifh; ifh = ifh->f_next)
		if (strcmp(ifhname, ifh->f_name) == 0 ||
		    strcmp(ofhname, ifh->f_name) == 0) {
			error(t, "", IFH);
			return;
		}
	/*
	 * The output file handle name is the contact name the user end
	 * is listening for, so send it.
	 */
	(void)sprintf(rfc, "%s/%d/%s", CHRFCADEV, useraddr, ofhname);
	if ((fd = open(rfc, 2)) < 0 ||
	    ioctl(fd, CHIOCSWAIT, CSRFCSENT) < 0 ||	/* Hangs here */
	    ioctl(fd, CHIOCGSTAT, &chst) < 0 ||
	    chst.st_state != CSOPEN ||
	    ioctl(fd, CHIOCSMODE, CHRECORD)) {
		if (fd >= 0)
			(void)close(fd);
		error(t, "", CNO);
		return;
	}
	if ((ifh = salloc(file_handle)) == FNULL ||
	    (ofh = salloc(file_handle)) == FNULL)
		fatal(NOMEM);
	ifh->f_fd = fd;
	ifh->f_type = F_INPUT;
	ifh->f_name = savestr(ifhname);
	ifh->f_xfer = XNULL;
	ifh->f_other = ofh;
	ifh->f_next = file_handles;
	file_handles = ifh;
	ofh->f_fd = fd;
	ofh->f_type = F_OUTPUT;
	ofh->f_name = savestr(ofhname);
	ofh->f_xfer = XNULL;
	ofh->f_other = ifh;
	ofh->f_next = file_handles;
	file_handles = ofh;
	respond(t, NOSTR);
}
/*
 * Close the data connection indicated by the file handle.
 */
undataconn(t)
register struct transaction *t;
{
	register struct file_handle *f, *of;

	f = t->t_fh;
	if (f->f_xfer != XNULL || f->f_other->f_xfer != XNULL)
		error(t, f->f_name, IFH);
	else {
		(void)close(f->f_fd);
		if (file_handles == f)
			file_handles = f->f_next;
		else {
			for (of = file_handles; of->f_next != f;
							of = of->f_next)
				if (of->f_next == FNULL)
					fatal(BADFHLIST);
			of->f_next = f->f_next;
		}
		if (file_handles == f->f_other)
			file_handles = f->f_other->f_next;
		else {
			for (of = file_handles; of->f_next != f->f_other;
							of = of->f_next)
				if (of->f_next == FNULL)
					fatal(BADFHLIST);
			of->f_next = f->f_other->f_next;
		}
		free(f->f_name);
		of = f->f_other;
		free((char *)f);
		free(of->f_name);
		free((char *)of);
		respond(t, NOSTR);
	}
}
/*
 * Open a file.
 * Unless a probe is specified, a transfer is created, and started.
 */
fileopen(t)
register struct transaction *t;
{
	register struct file_handle *f = t->t_fh;
	int errcode, fd;
	int options = t->t_args->a_options;
	char *pathname = t->t_args->a_string[0];
	char *realname = NOSTR, *dirname = NOSTR, *qfasl = NIL;
	char response[CHMAXDATA + 1];
	long nbytes;
	struct tm *tm;

	if ((errstring = parsepath(pathname, &dirname, &realname)) != NOSTR) {
		errcode = RAN;
		goto openerr;
	}
	if (options & (O_NEWOK|O_OLD|O_FAST)) {
		syslog(LOG_ERR, "FILE:UOO: 0%o\n", options);
		errcode = UOO;
		goto openerr;
	}
	errcode = 0;
	switch (options & (O_READ|O_WRITE|O_PROBE)) {
	case O_READ:
		if (f == FNULL || f->f_type != F_INPUT)
			errcode = IFH;
		break;
	case O_WRITE:
		if (f == FNULL || f->f_type != F_OUTPUT)
			errcode = IFH;
		break;
	case O_PROBE:
		if (f != FNULL)
			errcode = IFH;
		break;
	default:
		if (f == FNULL)
			options |= O_PROBE;
		else if (f->f_type == F_INPUT)
			options |= O_READ;
		else
			options |= O_WRITE;
	}
	if (errcode == 0 && f != NULL && f->f_xfer != XNULL)
		errcode = IFH;
	if (errcode != 0)
		goto openerr;
	switch (options & (O_PROBE|O_READ|O_WRITE)) {
	case O_PROBE:
		if (stat(realname, &sbuf) < 0) {
			errcode = FNF;
			switch (errno) {
			case EPERM:
				errstring = PERMDIR;
				break;
			case ENOTDIR:
				errstring = PATHNOTDIR;
				break;
			case ENOENT:
				if (access(dirname, 0) != 0)
					errcode = NSD;
				break;
			default:
				errcode = NER;
			}
		}
		if (options & O_DEFAULT)
			options |= O_CHARACTER;		/* Is this needed? */
		break;
	case O_WRITE:
		if ((fd = creat(realname, 0666)) < 0) {
			errcode = FNF;
			switch(errno) {
			case EPERM:
				if (access(dirname, 1) < 0)
					errstring = PERMDIR;
				else if (access(dirname, 2) < 0)
					errstring = WRITEDIR;
				else
					errstring = PERMFILE;
				break;
			case ENOENT:
				errcode = NSD;
				break;
			case ENOTDIR:
				errstring = PATHNOTDIR;
				break;
			default:
				errcode = NER;
			}
		} else if (fstat(fd, &sbuf) < 0)
			fatal(FSTAT);
		if (options & O_DEFAULT)
			options |= O_CHARACTER;		/* Is this needed? */
		break;
	case O_READ:
		if ((fd = open(realname, 0)) < 0) {
			errcode = FNF;
			switch (errno) {
			case EPERM:
				if (access(realname, 0) == 0)
					errstring = PERMFILE;
				else
					errstring = PERMDIR;
				break;
			case ENOENT:
				if (access(dirname, 0) < 0)
					errcode = NSD;
				break;
			case ENOTDIR:
				errstring = PATHNOTDIR;
				break;
			default:
				errcode = NER;
			}
		} else if (fstat(fd, &sbuf) < 0)
			fatal(FSTAT);
		else if (options & (O_BINARY | O_DEFAULT)) {
			long l;

			if (read(fd, (char *)&l, sizeof(l)) == sizeof(l) &&
			    l == QFASL) {
				qfasl = T;
				if (options & O_DEFAULT)
					options |= O_BINARY;
			} else if (options & O_DEFAULT)
				options |= O_CHARACTER;
			(void)lseek(fd, 0L, 0);
		}
		break;
	}
	if (errcode != 0) {
		if (errcode == NER)
			errstring = sys_errlist[errno];
		goto openerr;
	}
	tm = localtime(&sbuf.st_mtime);
	nbytes = options & O_CHARACTER ? sbuf.st_size :
		 sbuf.st_size / 2;
	if (protocol > 0)
		(void)sprintf(response, "%02d/%02d/%02d %02d:%02d:%02d %D %s%s%c%s%c",
			tm->tm_mon+1, tm->tm_mday, tm->tm_year,
			tm->tm_hour, tm->tm_min, tm->tm_sec, nbytes,
			qfasl, options & O_DEFAULT ? (options & O_CHARACTER ? " T" : " NIL")
			: "", CHNL, realname, CHNL);
	else
		(void)sprintf(response, "%d %02d/%02d/%02d %02d:%02d:%02d %D %s%c%s%c",
			-1, tm->tm_mon+1, tm->tm_mday, tm->tm_year,
			tm->tm_hour, tm->tm_min, tm->tm_sec, nbytes,
			qfasl, CHNL, realname, CHNL);
	if (options & (O_READ|O_WRITE))
		errcode = makexfer(t, options, fd, realname);
	if (errcode == 0)
		respond(t, response);
openerr:
	if (errcode)
		error(t, f != FNULL ? f->f_name : "", errcode);
	if (dirname)
		free(dirname);
	if (realname)
		free(realname);
}
/*
 * Make a transfer task and start it up.
 */
makexfer(t, options, fd, realname)
register struct transaction *t;
char *realname;
{
	register struct xfer *x;
	register int errcode;

	if ((x = salloc(xfer)) == XNULL)
		fatal(NOMEM);
	x->x_state = X_PROCESS;
	x->x_fh = t->t_fh;
	t->t_fh->f_xfer = x;
	x->x_work = TNULL;
	x->x_options = options;
	x->x_flags = 0;
	x->x_fd = fd;
	x->x_realname = savestr(realname);
	if (options & O_WRITE)
		x->x_room = BSIZE;
	else
		x->x_room = CHMAXDATA;
	x->x_bptr = x->x_bbuf;
	x->x_pptr = x->x_pbuf;
	x->x_left = 0;
	x->x_glob = (char **)0;
	x->x_next = xfers;
	xfers = x;
	if ((errcode = startxfer(x, t)) != 0)
		xflush(x);
	return errcode;
}
/*
 * Enqueue the transaction on its xfer.
 */
xcommand(t)
register struct transaction *t;
{
	register struct file_handle *f;
	register struct xfer *x;

	syslog(LOG_INFO, "FILE: transfer command: %s\n", t->t_command->c_name);
	if ((f = t->t_fh) == FNULL || (x = f->f_xfer) == XNULL)
		error(t, f->f_name, CNO);
	else {
#ifdef SELECT
		xqueue(t, x);
#else
		sendcmd(t, x);
#endif
	}
}
/*
 * Queue up the transaction onto the transfer.
 */
xqueue(t, x)
register struct transaction *t;
register struct xfer *x;
{
	register struct transaction **qt;

	for (qt = &x->x_work; *qt; qt = &(*qt)->t_next)
		;
	t->t_next = TNULL;
	*qt = t;
}
/*
 * Flush the transfer - just make the file handle not busy
 */
xflush(x)
register struct xfer *x;
{
	register struct xfer **xp;

	for (xp = &xfers; *xp && *xp != x; xp = &(*xp)->x_next)
		;
	if (*xp == XNULL)
		fatal("Missing xfer");
	*xp = x->x_next;
	x->x_fh->f_xfer = XNULL;
	xfree(x);
}
/*
 * Free storage associated with xfer struct.
 */
xfree(x)
register struct xfer *x;
{
	register struct transaction *t;

	while ((t = x->x_work) != TNULL) {
		x->x_work = t->t_next;
		tfree(t);
	}
	if (x->x_realname)
		free(x->x_realname);
	if (x->x_glob)
		blkfree(x->x_glob);
	sfree(x);
}

/*
 * Perform the close on the transfer.
 */
fileclose(t)
register struct transaction *t;
{
	register struct xfer *x;

	char response[CHMAXDATA];
	struct tm *tm;

	if (t->t_fh == FNULL || (x = t->t_fh->f_xfer) == XNULL) {
		error(t, t->t_fh->f_name, CNO);
		return;
	}
	syslog(LOG_INFO, "FILE: doing CLOSE\n");
	x->x_state = X_DONE;
	if (x->x_options & O_DIRECTORY)
		respond(t, NOSTR);
	else if (fstat(x->x_fd, &sbuf) < 0)
		fatal(FSTAT);
	else {
		tm = localtime(&sbuf.st_mtime);
		if (protocol > 0)
			(void)
			sprintf(response, "%02d/%02d/%02d %02d:%02d:%02d %D%c%s%c",
				tm->tm_mon+1, tm->tm_mday, tm->tm_year,
				tm->tm_hour, tm->tm_min, tm->tm_sec, sbuf.st_size,
				CHNL, x->x_realname, CHNL);
		else
			(void)
			sprintf(response, "%d %02d/%02d/%02d %02d:%02d:%02d %D%c%s%c",
				-1, tm->tm_mon+1, tm->tm_mday, tm->tm_year,
				tm->tm_hour, tm->tm_min, tm->tm_sec, sbuf.st_size,
				CHNL, x->x_realname, CHNL);
		respond(t, response);
		close(x->x_fd);
	}
#ifndef SELECT
	/*
	 * After responding to the close, we tell the transfer process to
	 * finish.  In a read, this means sending a synchronous mark.
	 * In a write, it means consuming a synchronous mark.
	 * If we are writing, and have aborted the transfer, the data in the
	 * close will not necessarily reflect the final state of the file.
	 * We "hope" that, in the normal case of writing, that the transfer
	 * process has enough time to flush its disk buffer after receiving
	 * an EOF before we receive the CLOSE and do the fstat.  See dowork.
	 */
	kill(x->x_pid, SIGHUP);
#endif
}
/*
 * Check and enqueue a filepos command.
 */
filepos(x, t)
register struct xfer *x;
register struct transaction *t;
{
	if (x->x_state != X_PROCESS)
		error(t, x->x_fh->f_name, CNO);
	else if ((x->x_options & O_READ) == 0)
		error(t, x->x_fh->f_name, IFH);
	else if (syncmark(x->x_fh) < 0) {
		errstring = "Data connection error";
		error(t, x->x_fh->f_name, RAN);
	} else {
		x->x_left = 0;
		lseek(x->x_fd, t->t_args->a_num[0], 0);
		respond(t, NOSTR);
	}
}
/*
 * Continue a transfer which is in the asynchronously marked state.
 */
filecontinue(x, t)
register struct xfer *x;
register struct transaction *t;
{
	if (x->x_state != X_ERROR) {
		errstring = "CONTINUE received when not in error state.";
		error(t, x->x_fh->f_name, IOC);
	} else {
		x->x_state = X_PROCESS;
		respond(t, NOSTR);
	}
}

/*
 * Delete the given file.
 */
delete(t)
register struct transaction *t;
{
	register struct file_handle *f = t->t_fh;
	char *file, *dir = NOSTR, *real = NOSTR;
	struct stat sbuf;
	int errcode;

	if (f != FNULL)
		if (t->t_args != ANULL && t->t_args->a_string[0] != NOSTR)
			error(t, f->f_name, IRF);		
		else if (f->f_xfer == XNULL)
			error(t, f->f_name, CNO);
		else if (f->f_xfer->x_options & O_DIRECTORY)
			error(t, f->f_name, CNO);
		else if (unlink(f->f_xfer->x_realname) < 0)
			goto badunlink;
		else
			respond(t, NOSTR);
	else if (t->t_args == ANULL ||
		 (file = t->t_args->a_string[0]) == NOSTR)
		error(t, "", IRF);
	else if ((errstring = parsepath(file, &dir, &real)) != NOSTR)
		error(t, "", RAN);
	else if (stat(real, &sbuf) < 0) {
		errcode = RAN;
		switch (errno) {
		case EPERM:
			errstring = PERMDIR;
			break;
		case ENOENT:
			errstring = MISSDIR;
			break;
		default:
			errstring = sys_errlist[errno];
		}
		error(t, "", errcode);
	} else if ((sbuf.st_mode & S_IFMT) == S_IFDIR) {
		if (access(dir, 3) != 0) {	/* Can't search or write containing dir */
			errstring = "No search or write permission on containing directory.";
			error(t, "", errcode);
		} else if (access(real, 3) != 0) {	/* Can't search or write dir */
			errstring = "No search or write permission on directory to be deleted.";
			error(t, "", errcode);
		} else {
			register int pid, rp;
			int st;

			if ((pid = fork()) == 0) {
				close(0);
				close(1);
				close(2);
				open("/dev/null", 2);
				dup(0); dup(0);
				execl("/bin/rmdir", "rmdir", real, 0);
				execl("/usr/bin/rmdir", "rmdir", real, 0);
				exit(1);
			}
			while ((rp = wait(&st)) >= 0)
				if (rp != pid)
					nprocdone++;
				else
					break;
			if (rp != pid)
				fatal("Lost a process!");
			if (st != 0) {
				errstring = "Directory to be deleted is not empty.";
				error(t, "", errcode);
			} else
				respond(t, NOSTR);
		}
	} else if (unlink(real) < 0) {
badunlink:
		errcode = RAN;
		switch (errno) {
		case EPERM:
			if (access(dir, 1) == 0)
				errstring = WRITEDIR;
			else
				errstring = PERMDIR;
			break;
		case ENOENT:
			if (access(dir, 1) == 0)
				errcode = FNF;
			else
				errstring = MISSDIR;
			break;
		default:
			errstring = sys_errlist[errno];
		}
		error(t, "", errcode);
	} else
		respond(t, NOSTR);
	if (dir != NOSTR)
		free(dir);
	if (real != NOSTR)
		free(real);
}	
/*
 * Rename a file.
 */
rename(t)
register struct transaction *t;
{
	register struct file_handle *f;
	register struct xfer *x;
	char *file1, *file2, *dir1 = NOSTR, *dir2 = NOSTR,
		*real1 = NOSTR, *real2 = NOSTR;

	file1 = t->t_args->a_string[0];
	file2 = t->t_args->a_string[1];
	if ((errstring = parsepath(file1, &dir1, &real1)) != NOSTR)
		error(t, "", RAN);
	else if ((f = t->t_fh) != FNULL)
		if (file2 != NOSTR)
			error(t, f->f_name, IRF);
		else if ((x = f->f_xfer) == XNULL)
			error(t, f->f_name, IFH);
		else if (x->x_options & (O_READ|O_WRITE))
			if ((errstring = mv(x->x_realname, real1)) != NOSTR)
				error(t, f->f_name, RAN);
			else {
				free(x->x_realname);
				x->x_realname = real1;
				real1 = NOSTR;
				respond(t, NOSTR);
			}
		else {
			errstring = "Can't rename in DIRECTORY command.";
			error(t, f->f_name, RAN);
		}			
	else if (file2 == NOSTR)
		error(t, "", IRF);
	else if ((errstring = parsepath(file2, &dir2, &real2)) != NOSTR)
		error(t, "", RAN);
	else if ((errstring = mv(real1, real2)) != NOSTR)
		error(t, "", RAN);
	else
		respond (t, NOSTR);
	if (dir1)
		free(dir1);
	if (dir2)
		free(dir2);
	if (real1)
		free(real1);
	if (real2)
		free(real2);
}
		
/*
 * Parse the pathname into directory and full name parts.
 * Returns text messages if anything is wrong.
 * Checking is done for well-formed pathnames, .. components,
 * and leading tildes.  Wild carding is not done here. Should it?
 */
char *
parsepath(path, dir, real)
register char *path;
char **dir, **real;
{
	register char *cp;
	char *wd, save;

	if (*path == '~') {
		for (cp = path; *cp != '\0' && *cp != '/'; cp++)
			;
		if (cp == path + 1) {
			if ((wd = home) == NOSTR)
				return "Can't expand '~', not logged in.";
		} else {
			struct passwd *pw;

			save = *cp;
			*cp = '\0';
			if ((pw = getpwnam(path+1)) == NULL) {
				*cp = save;
				return "Unknown user name after '~'.";
			}
			*cp = save;
			wd = pw->pw_dir;
		}
		path = cp;
		while (*path == '/')
			path++;
	} else if (*path == '/')
		wd = "";
	else if ((wd = cwd) == NOSTR)
		return "There is no working directory.";
	if ((cp = malloc((unsigned)(strlen(wd) + strlen(path) + 2))) == NOSTR)
		fatal(NOMEM);
	strcpy(cp, wd);
	if (wd[0] != '\0')
		(void)strcat(cp, "/");
	(void)strcat(cp, path);
	dcanon(cp);
	*real = cp;
	if ((cp = rindex(cp, '/')) == NOSTR)
		fatal("Parsepath");
	if (cp == *real)
		cp++;
	save = *cp;
	*cp = '\0';
	*dir = savestr(*real);
	*cp = save;
	return NOSTR;
}
/*
 * dcanon - canonicalize the pathname, removing excess ./ and ../ etc.
 *	we are of course assuming that the file system is standardly
 *	constructed (always have ..'s, directories have links)
 * Stolen from csh (My old code).
 */
dcanon(cp)
	char *cp;
{
	register char *p, *sp;
	register int slash;

	if (*cp != '/')
		fatal("dcanon");
	for (p = cp; *p; ) {		/* for each component */
		sp = p;			/* save slash address */
		while(*++p == '/')	/* flush extra slashes */
			;
		if (p != ++sp)
			strcpy(sp, p);
		p = sp;			/* save start of component */
		slash = 0;
		while(*++p)		/* find next slash or end of path */
			if (*p == '/') {
				slash = 1;
				*p = 0;
				break;
			}
		if (*sp == '\0')	/* if component is null */
			if (--sp == cp)	/* if path is one char (i.e. /) */
				break;
			else
				*sp = '\0';
		else if (sp[0] == '.' && sp[1] == '\0') {
			if (slash) {
				strcpy(sp, ++p);
				p = --sp;
			} else if (--sp != cp)
				*sp = '\0';
		} else if (sp[0] == '.' && sp[1] == '.' && sp[2] == '\0') {
			if (--sp != cp)
				while (*--sp != '/')
					;
			if (slash) {
				strcpy(++sp, ++p);
				p = --sp;
			} else if (cp == sp)
				*++sp = '\0';
			else
				*sp = '\0';
		} else if (slash)
			*p = '/';
	}
}
/*
 * Process a login command... verify the users name and
 * passwd.
 */
login(t)
register struct transaction *t;
{
	register struct passwd *p;
	register struct cmdargs *a;
	char *name;
	char response[CHMAXDATA];

	a = t->t_args;
	if ((name = a->a_string[0]) == NOSTR) {
		syslog(LOG_ERR, "FILE exiting due to logout\n");
		finish();
	} else if (home != NOSTR) {
		errstring = "You are already logged in.";
		error(t, "", RAN);
	} else if ((p = getpwnam(name)) == (struct passwd *)0) {
		errstring = "Login incorrect.";
		error(t, "", UNK);
	} else if (p->pw_passwd != NOSTR && *p->pw_passwd != '\0' &&
		   (a->a_string[1] == NOSTR ||
		    strcmp(crypt(a->a_string[1], p->pw_passwd), p->pw_passwd) != 0)) {
		errstring = "Login incorrect.";
		error(t, "", UNK);
	} else {
		home = savestr(p->pw_dir);
		cwd = savestr(home);
		setuid(p->pw_uid);
		setgid(p->pw_gid);
		(void)sprintf(response, "%s %s/%c%s%c", p->pw_name, p->pw_dir,
				   CHNL, fullname(p), CHNL);
		respond(t, response);
		syslog(LOG_INFO, "FILE: logged in by %s\n", p->pw_name);
	}
}
/*
 * Generate the full name from the password file entry, according to
 * the current "finger" program conventions.  This code was sort of
 * lifted from finger and needs to be kept in sync with it...
 */
char *
fullname(p)
register struct passwd *p;
{
	static char fname[100];
	register char *cp;
	register char *gp;

	gp = p->pw_gecos;
	cp = fname;
	if (*gp == '*')
		gp++;
	while (*gp != '\0' && *gp != ',') {
		if( *gp == '&' )  {
			char *lp = p->pw_name;

			if (islower(*lp))
				*cp++ = toupper(*lp++);
			while (*lp)
				*cp++ = *lp++;
		} else
			*cp++ = *gp;
		gp++;
	}
	*cp = '\0';
	for (gp = cp; gp > fname && *--gp != ' '; )
		;
	if (gp == fname)
		return fname;
	*cp++ = ',';
	*cp++ = ' ';
	*gp = '\0';
	strcpy(cp, fname);
	return ++gp;
}
/*
 * Implement the directory command.
 */
directory(t)
register struct transaction *t;
{
	register struct cmdargs *a = t->t_args;
	int errcode = 0;
	char *dirname = NOSTR, *realname = NOSTR;

	if (a->a_options & ~(O_DELETED|O_FAST)) {
		syslog(LOG_ERR, "FILE:UOO: 0%o\n", a->a_options);
		errcode = UOO;
		goto direrr;
	}
	if (t->t_fh->f_xfer != XNULL) {
		errcode = IFH;
		goto direrr;
	}
	if (errstring = parsepath(a->a_string[0], &dirname, &realname)) {
		errcode = RAN;
		goto direrr;
	}
	/*
	 * Note that we don't respond here.  We wait until we are sure that
	 * we have at least one file to describe given the potentially
	 * wild-card pathname supplied.  We don't do the wild-card expansion
	 * here it can take some time, and that would violate the asynchronous
	 * nature of the program as a whole.  We make this work by queueing
	 * the DIRECTORY transaction on the transfer, so that when it is
	 * started it will first do the glob and respond then.
	 */
	if (!(a->a_options & (O_BINARY | O_CHARACTER)))
		a->a_options |= O_CHARACTER;
	errcode = makexfer(t, a->a_options | O_DIRECTORY, 0, realname);
direrr:
	if (errcode)
		error(t, t->t_fh->f_name, errcode);
	if (dirname)
		free(dirname);
	if (realname)
		free(realname);
}
/*
 * Start up a directory transfer by first doing the glob and responding to the
 * dirctory command.
 */
dirstart(x, t)
register struct xfer *x;
register struct transaction *t;
{
	register int n;

	if ((x->x_glob = glob(x->x_realname)) != (char **)0) {
		for (x->x_gptr = x->x_glob; *x->x_gptr; x->x_gptr++) {
			if ((n = stat(*x->x_gptr, &sbuf)) == 0)
				break;
		}
		if (*x->x_gptr) {
			respond(t, "");
			/*
			 * We create the firest record to be returned, which
			 * is the properties of the file system as a whole.
			 */
			(void)
			sprintf(x->x_bbuf, "\nSETTABLE-PROPERTIES AUTHOR CREATION-DATE REFERENCE-DATE PROTECTION-MODE\nBLOCK-SIZE %d\n\n",
				BSIZE);
			n = strlen(x->x_bbuf);
			x->x_bptr = x->x_bbuf;
			x->x_left = n;
			return;
		}
		sfree(x->x_glob);
	}
	error(t, t->t_fh->f_name, FNF);
	x->x_state = X_BROKEN;
}
/*
 * Assemble a directory entry record in the buffer for this transfer.
 */
direntry(x, s)
register struct xfer *x;
register struct stat *s;
{
	register struct passwd *pw;
	struct tm *tm;

	x->x_bptr = x->x_bbuf;
	if (x->x_options & O_FAST) {
		(void)sprintf(x->x_bptr, "%s\n\n", *x->x_gptr);
		x->x_bptr += strlen(x->x_bptr);
	} else {
		(void)
		sprintf(x->x_bbuf, "%s\nPHYSICAL-VOLUME %d\nBLOCK-SIZE %d\n",
			*x->x_gptr, s->st_dev, BSIZE);
		x->x_bptr += strlen(x->x_bptr);
		(void)
		sprintf(x->x_bptr, "LENGTH-IN-BLOCKS %D\nBYTE-SIZE 8\n",
			(s->st_size + BSIZE - 1) / BSIZE);
		x->x_bptr += strlen(x->x_bptr);
		(void)
		sprintf(x->x_bptr, "LENGTH-IN-BYTES %D\n",
			s->st_size);
		x->x_bptr += strlen(x->x_bptr);
		tm = localtime(&s->st_mtime);
		(void)
		sprintf(x->x_bptr, "CREATION-DATE %02d/%02d/%02d %02d:%02d:%02d\n",
			tm->tm_mon+1, tm->tm_mday, tm->tm_year,
			tm->tm_hour, tm->tm_min, tm->tm_sec);
		x->x_bptr += strlen(x->x_bptr);
		tm = localtime(&s->st_atime);
		(void)
		sprintf(x->x_bptr, "REFERENCE-DATE %02d/%02d/%02d %02d:%02d:%02d\n",
			tm->tm_mon+1, tm->tm_mday, tm->tm_year,
			tm->tm_hour, tm->tm_min, tm->tm_sec);
		x->x_bptr += strlen(x->x_bptr);
		if ((s->st_mode & S_IFMT) == S_IFDIR) {
			(void)sprintf(x->x_bptr, "DIRECTORY T\n");
			x->x_bptr += strlen(x->x_bptr);
		}
		if (pw = getpwuid(s->st_uid))
			(void) sprintf(x->x_bptr, "AUTHOR %s\n\n", pw->pw_name);
		else
			(void) sprintf(x->x_bptr, "AUTHOR #%d\n\n", s->st_uid);
		x->x_bptr += strlen(x->x_bptr);
	}
}
	
/*
 * File name completion.
 * Completion should allow wild carding, but what to do when multiple files
 * match??
 * This requires more cooperation with the pathname package wrt building
 * string-for-host for default pathnames.
 */
complete(t)
register struct transaction *t;
{
	char buf[10];

	respond(t, sprintf(buf, " NIL%c%c", CHNL, CHNL));
}
/*
 * Change properties.
 */
changeprop(t)
register struct transaction *t;
{
	char *dir, *file;
	struct stat sbuf;

	if (errstring = parsepath(t->t_args->a_string[0], &dir, &file))
		error(t, "", RAN);
	else if (stat(file, &sbuf) < 0)
		error(t, "", FNF);
	else {
		errstring = "Changing properties is not implemented.";
		error(t, "", RAN);
	}
}
/*
 * Create a directory
 */
crdir(t)
register struct transaction *t;
{
	char *dir = NULL, *file = NULL, *parent = NULL;
	struct stat sbuf;

	if (errstring = parsepath(t->t_args->a_string[0], &dir, &file))
		error(t, "", RAN);
	else {
		free(file);
		file = NULL;
		if (errstring = parsepath(dir, &parent, &file))
			error(t, "", RAN);
		else if (access(parent, 3) != 0) {
			if (errno == EPERM)
				errstring = "Permission denied on parent directory.";
			else
				errstring = "Parent directory doesn't exist.";
			error(t, "", RAN);
		} else if (stat(dir, &sbuf) >= 0) {
			if ((sbuf.st_mode & S_IFMT) == S_IFDIR)
				errstring = "Directory already exists."	;
			else
				errstring = "File already exists with same name.";
			error(t, "", RAN);
		} else {
			register int pid, rp;
			int st;

			if ((pid = fork()) == 0) {
				close(0);
				close(1);
				close(2);
				open("/dev/null", 2);
				dup(0); dup(0);
				execl("/bin/mkdir", "mkdir", dir, 0);
				execl("/usr/bin/mkdir", "mkdir", dir, 0);
				exit(1);
			}
			while ((rp = wait(&st)) >= 0)
				if (rp != pid)
					nprocdone++;
				else
					break;
			if (rp != pid)
				fatal("Lost a process!");
			if (st != 0) {
				errstring = "UNIX mkdir failed.";
				error(t, "", RAN);
			} else
				respond(t, NOSTR);
		}
	}
	if (file)
		free(file);
	if (dir)
		free(dir);
	if (parent)
		free(parent);
}

/*
 * mv, move one file to a new name (works for directory)
 * Second name must not exist.
 * Errors are returned in strings.
 */
char *
mv(from, to)
char *from, *to;
{
	struct stat sbuf;

	if (stat(from, &sbuf) < 0)
		if (errno == EPERM)
			return PERMDIR;
		else if (errno == ENOTDIR)
			return PATHNOTDIR;
		else
			return "Source file of rename not found";
	else if (stat(to, &sbuf) >= 0)
		return "Target file name already exists.";
	else if (errno == EPERM)
		return PERMDIR;
	else if (errno == ENOTDIR)
		return PATHNOTDIR;
	else if (errno != ENOENT)
		return sys_errlist[errno];
	if (link(from, to) < 0)
		if (errno == ENOENT)
			return "Directory non-existent in target name";
		else if (errno == EXDEV)
			return "Can't rename across UNIX file systems.";
		else if (errno == EPERM)
			return "Permission denied for creating new name.";
		else
			return sys_errlist[errno];
	if (unlink(from) < 0) {
		(void)
		unlink(to);
		return "Permission denied on removing old name";
	}
	return NOSTR;
}
/*
 * File handle error routine.
 */
fherror(f, code, type, message)
register struct file_handle *f;
char *message;
{
	register struct file_error *e = &errors[code];
	struct chpacket pkt;
	register int len;

	pkt.cp_op = ASYNOP;
	(void)sprintf(pkt.cp_data, "TIDNO %s ERROR %s %c %s",
		f->f_name, e->e_code, type, message ? message : e->e_string);
	len = 1 + strlen(pkt.cp_data);
	if (f->f_type == F_INPUT) {
		if (write(1, (char *)&pkt, len) != len) 
			fatal(CTLWRITE);
	} else if (write(f->f_fd, (char *)&pkt, len) != len) {
		f->f_xfer->x_state = X_BROKEN;
	}
}
/*
 * Function to perform some work on a transfer.
 * If non zero is returned, the transfer can be flushed.
 */
dowork(x)
register struct xfer *x;
{
	register struct transaction *t;
	register int n;

	/*
	 * First, do any queued transactions, all of which are
	 * short things to do (continue, set-pointer etc.).
	 */
	while ((t = x->x_work) != TNULL) {
		(*t->t_command->c_func)(x, t);
		x->x_work = t->t_next;
	}
	/*
	 * If the connection was broken off, just flush the transfer here.
	 */
	if (x->x_state == X_BROKEN)
		return X_FLUSH;
	/*
	 * The only way to get out of the ERROR (recoverable) or
	 * ABORT (fatal) states if for the user end to either
	 * do a CONTINUE (in the recoverable state) or a CLOSE
	 * in either state, or to shut down the control connection.
	 */
	if (x->x_state == X_HUNG || x->x_state == X_ABORT || x->x_state == X_ERROR)
		return X_HANG;
	/*
 	 * Now, do a unit of work on the transfer, which should be one
	 * packet read or written to the network channel.
	 * Note that this only provides task multiplexing as far as use of the
	 * network goes, not anything else.  However the non-network work
	 * is usually just a single disk operation. 
	 */
	switch (x->x_options & (O_READ | O_WRITE | O_DIRECTORY)) {
	case O_READ:
	case O_DIRECTORY:
		if (x->x_state == X_DONE) {
			/*
			 * After the transfer, send a synchronous mark to
			 * end it at the other end.
			 */
			(void)syncmark(x->x_fh);	/* Ignore errors */
			return X_FLUSH;
		}
		if (x->x_flags & X_EOF) {
			if (xpweof(x) < 0)
				return X_FLUSH;
			x->x_state = X_HUNG;
			return X_HANG;
		}
		/*
		 * Fill the net packet, send it, return.
		 */
		while (x->x_room != 0) {
			if (x->x_left == 0) {
				if (x->x_options & O_DIRECTORY) {
					while (*x->x_gptr &&
					       stat(*x->x_gptr, &sbuf) < 0)
						x->x_gptr++;
					if (*x->x_gptr == NOSTR)
						n = 0;
					else {
						direntry(x, &sbuf);
						n = strlen(x->x_bbuf);
						x->x_gptr++;
					}
				} else
					n = read(x->x_fd, x->x_bbuf, BSIZE);
				syslog(LOG_INFO, "FILE: read %d bytes from file\n",
					n);
				if (n <= 0) {
					if (n == 0) {
						if (xpwrite(x) < 0)
							return X_FLUSH;
						x->x_flags |= X_EOF;
					} else {
						x->x_state = X_ABORT;
						fherror(x->x_fh, IOC, E_FATAL,
							sys_errlist[errno]);
					}
					return X_CONTINUE;
				}
				x->x_left = n;
				x->x_bptr = x->x_bbuf;
			}
			switch (x->x_options & (O_CHARACTER|O_RAW|O_SUPER)) {
			case 0:
			case O_CHARACTER | O_RAW:
				n = MIN(x->x_room, x->x_left);
				bmove(x->x_bptr, x->x_pptr, n);
				x->x_left -= n;
				x->x_bptr += n;
				x->x_room -= n;
				x->x_pptr += n;
				break;
			case O_CHARACTER:	/* default ascii */
				to_lispm(x);
				break;
			case O_CHARACTER | O_SUPER:
				to_lispm(x);	/* for now, same as ascii */
				break;
			}
		}
		if (xpwrite(x) < 0)
			return X_FLUSH;
		return X_CONTINUE;
	case O_WRITE:
		if (x->x_state == X_DONE)
			/*
			 * We got the EOF or have been CLOSE'd, just wait for
			 * the synchronous mark to flush the transfer.
			 */
			if (x->x_flags & X_SYNOP ||
			    xpread(x) < 0 || (x->x_op & 0377) == SYNOP)
				return X_FLUSH;
			else
				/*
				 * We evidently are getting more packets
				 * after being X_DONE.  They are either
				 * random packets after the EOF
				 * (User end protocol botch) or random
				 * packets in the net after a user CLOSE.
				 */
				return X_CONTINUE;
		/*
		 * In case we returned last time due to a disk write
		 * error, we must try again before reading from the
		 * network again.
		 */
		if (x->x_room == 0)
			if (xbwrite(x) < 0)
				return X_CONTINUE;
		/*
		 * If we have read an EOF, we are done.  If we get an error
		 * flushing the last disk buffer, we will come through here
		 * again, so we set the EOF flag to avoid reading from the
		 * connection any more.
		 * Note that when we read the EOF packet, the other end
		 * gets acknowledgement saying, in effect that we have
		 * completed the transfer. In the (impossibly?) rare case that
		 * we read an EOF (acking it), and the other end receives the
		 * ack, sends a close and we receive the close, all before we
		 * have written the last disk buffer, the information in the
		 * close response will be wrong and/or the file will not be
		 * completely written.
		 */
		if (x->x_flags & X_EOF || (n = xpread(x)) == 0) {
			x->x_flags |= X_EOF;
			if (xbwrite(x))
				return X_CONTINUE;
			/*
			 * Here we need to wait for a close before waiting
			 * for the SYNOP.
			 */
			x->x_state = X_HUNG;
			return X_CONTINUE;
		} else if (n == -1) {
			/*
			 * On a network read error, we can be a little
			 * friendlier than when reading, since we can still
			 * write back an error on the control connection.
			 */
			x->x_state = X_ABORT;
			fherror(x->x_fh, IOC, E_FATAL, "Data connection error");
			return X_CONTINUE;
		} else if (n == -2) {
			/*
			 * On an interrupted read, we just return.
			 */
			return X_CONTINUE;
		} else if ((x->x_op & 0377) == SYNOP) {
			/*
			 * If we get a synchronous mark before an EOF,
			 * it should be treated as both an EOF and synchronous
			 * mark together.  We don't bother to flush the disk
			 * buffer since no EOF was sent.
			 * SHOULDN'T HAPPEN.
			 */
			x->x_flags |= X_SYNOP;
			x->x_state = X_HUNG;
			return X_CONTINUE;
		}
		while (x->x_left) {
			switch (x->x_options & (O_CHARACTER|O_RAW|O_SUPER)) {
			case 0:
			case O_CHARACTER | O_RAW:
				n = MIN(x->x_left, x->x_room);
				bmove(x->x_pptr, x->x_bptr, n);
				x->x_pptr += n;
				x->x_left -= n;
				x->x_room -= n;
				x->x_bptr += n;
				break;
			case O_CHARACTER:
				from_lispm(x);
				break;
			case O_CHARACTER | O_SUPER:
				from_lispm(x);
				break;
			}
			if (x->x_room == 0)
				if (xbwrite(x))
					return X_CONTINUE;
		}
		return X_CONTINUE;
	}
	/* NOTREACHED */
}
/*
 * Character set conversion routines.
 */
to_lispm(x)
register struct xfer *x;
{
	register int c;

	while (x->x_left && x->x_room) {
		c = *x->x_bptr++ & 0377;
		x->x_left--;
		if (x->x_flags & X_QUOTED)
			switch (c) {
			default:
				c |= 0200;
			case 010:
			case 011:
			case 012:
			case 014:
			case 015:
			case 0177:
				x->x_flags &= ~X_QUOTED;
				*x->x_pptr++ = c;
				x->x_room--;
			}
		else if (c == 0177)
			x->x_flags |= X_QUOTED;
		else {
			switch (c) {
			case '\n':
				c = CHNL;
				break;
			case 010:
			case 011:
			case 013:
			case 014:
				c |= 0200;
			}
			*x->x_pptr++ = c;
			x->x_room--;
		}
	}
}
from_lispm(x)
register struct xfer *x;
{
	register int c;

	while (x->x_left && x->x_room) {
		c = *x->x_pptr & 0377;
		if (!(x->x_flags & X_QUOTED))
			switch (c) {
			default:
				if (!(c & 0200))
					break;
				*x->x_pptr &= ~0200;
			case 010:
			case 011:
			case 012:
			case 014:
			case 015:
			case 0177:
				*x->x_bptr++ = 0177;
				x->x_room--;
				x->x_flags |= X_QUOTED;
				continue;
			case 0210:
			case 0211:
			case 0212:
			case 0214:
				c &= ~0200;
				break;
			case 0215:
				c = '\n';
				break;
			}
		x->x_left--;
		x->x_options &= ~X_QUOTED;
		*x->x_bptr++ = c;
		x->x_room--;
		x->x_pptr++;
	}
}		

/*
 * Write out the local disk buffer, doing the appropriate error
 * processing here, returning non zero if we got an error.
 */
xbwrite(x)
register struct xfer *x;
{
	register int ret;
	register int n;

	if ((n = x->x_bptr - x->x_bbuf) == 0)
		return 0;
	for (;;) {
		if ((ret = write(x->x_fd, x->x_bbuf, n)) <= 0) {
			syslog(LOG_INFO, "FILE: write error %d (%d) to file\n",
				ret, errno);
			if (ret == 0 || errno == ENOSPC) {
				n = E_RECOVERABLE;
				x->x_state = X_ERROR;
			} else {
				n = E_FATAL;
				x->x_state = X_ABORT;
			}
			fherror(x->x_fh, IOC, n, sys_errlist[errno]);
			return -1;
		}
		syslog(LOG_INFO,"FILE: wrote %d bytes to file\n", ret);
		if (ret == n)
			break;
		/*
		 * In the (unlikely) case that we write less than we asked for.
		 */
		bmove(x->x_bbuf + ret, x->x_bbuf, ret);
		n -= ret;
		x->x_bptr -= ret;
	}
	x->x_bptr = x->x_bbuf;
	x->x_room = BSIZE;
	return 0;
}
/*
 * Write an eof packet on a transfer.
 */
xpweof(x)
register struct xfer *x;
{
	char op = EOFOP;

	if (write(x->x_fh->f_fd, &op, 1) != 1) {
		x->x_state = X_BROKEN;
		return -1;
	}
	syslog(LOG_INFO, "FILE: wrote EOF to net\n");
	return 0;
}
/*
 * Write a transfer's packet.
 */
xpwrite(x)
register struct xfer *x;
{
	register int len;

	len = x->x_pptr - x->x_pbuf;
	if (len == 0)
		return 0;
	x->x_op = x->x_options & O_BINARY ? DWDOP : DATOP;
	len++;
	syslog(LOG_INFO, "FILE: writing (%d) %d bytes to net\n",
		x->x_op & 0377, len);
	if (write(x->x_fh->f_fd, (char *)&x->x_pkt, len) != len) {
		x->x_state = X_BROKEN;
		return -1;
	}
	x->x_pptr = x->x_pbuf;
	x->x_room = CHMAXDATA;
	return 0;
}
/*
 * Read a packet from the net, returning 0 for EOF packets, < 0 for errors,
 * and > 0 for data.
 */
xpread(x)
register struct xfer *x;
{
	register int n;

	n = read(x->x_fh->f_fd, (char *)&x->x_pkt, sizeof(x->x_pkt));
	syslog(LOG_INFO, "FILE: read (%d) %d bytes from net\n",
		x->x_op & 0377, n);
	if (n < 0)
		return -1;
	if (n == 0)
		fatal("Net Read returns 0");
	switch (x->x_op & 0377) {
	case EOFOP:
		return 0;
	case DATOP:
	case DWDOP:
	case SYNOP:
		x->x_left = n - 1;
		x->x_pptr = x->x_pbuf;
		return 1;
	default:
		syslog(LOG_ERR, "FILE: bad opcode in data connection: %d\n",
			x->x_op & 0377);
		fatal("Bad opcode on data connection");
		/* NOTREACHED */
	}
}	
/*
 * Block move, (from, to, numbytes).
 */
bmove(from, to, n)
register char *from;
register char *to;
register int n;
{
	if (n)
		do *to++ = *from++; while (--n);
}
/*
 * End this program right here.
 * Nothing really to do since system closes everything.
 */
finish()
{
	/* Should send close packet here */
	exit(0);
}
/*
 * Start the transfer task running.
 * Returns errcode if an error occurred, else 0
 */
startxfer(ax, t)
struct xfer *ax;
register struct transaction *t;
{
	register struct xfer *x = ax;
#ifdef SELECT
put on runq or something, set up which fd and why to wait....
#else
	register int i;
	int pfd[2];

	if (pipe(pfd) < 0) {
		errstring = "Can't create pipe for transfer process";
		return NER;
	}
	x->x_pfd = pfd[1];
	switch (x->x_pid = fork()) {
	case -1:
		close(pfd[0]);
		close(pfd[1]);
		errstring = "Can't create transfer process";
		return NER;
	default:
		close(pfd[0]);
		return 0;
	/*
	 * Child process.
	 */
	case 0:
		for (i = 3; i < 20; i++)
			if (i != pfd[0] && i != x->x_fh->f_fd &&
			    i != ctlpipe[1] && i != x->x_fd)
				close(i);
		x->x_pfd = pfd[0];
		dosubproc(x, t);
		(void)
		write(ctlpipe[1], (char *)&ax, sizeof(x));
		syslog(LOG_INFO, "FILE: subproc exiting\n");
		exit(0);
	}
	/* NOTREACHED */
#endif
}
#ifndef SELECT


/*
 * Rather than queuing up a transaction on a transfer that is a subtask
 * of this process, send it to the subprocess.
 */
sendcmd(t, x)
register struct transaction *t;
register struct xfer *x;
{
	struct pmesg pm;

	pm.pm_cmd = t->t_command;
	strncpy(pm.pm_tid, t->t_tid, TIDLEN);
	pm.pm_tid[TIDLEN] = '\0';
	if (t->t_args) {
		pm.pm_args = 1;
		pm.pm_n = t->t_args->a_num[0];
	} else
		pm.pm_args = 0;
	if (write(x->x_pfd, (char *)&pm, sizeof(pm)) != sizeof(pm))
		error(t, t->t_fh->f_name, CNO);
	syslog(LOG_INFO, "FILE: send %s command\n", t->t_command->c_name);
}
/*
 * Check the control pipe to see if any transfer processes exited, and
 * pick them up.
 */
xcheck()
{
	register int nprocs;
	struct xfer *x;
	off_t nread = 0;

	if (ctlpipe[0] <= 0)
		if (pipe(ctlpipe) < 0)
			fatal("Can't create control pipe");
	nprocs = 0;
	if (ioctl(ctlpipe[0], FIONREAD, &nread) < 0)
		fatal("Failing FIONREAD on control pipe");
	while (nread) {
		if (read(ctlpipe[0], (char *)&x, sizeof(x)) != sizeof(x))
			fatal("Read error on control pipe");
		nprocs++;
		close(x->x_pfd);
		xflush(x);
		nread -= sizeof(x);
	}
	nprocs -= nprocdone;
	nprocdone = 0;
	while (nprocs--)
		if (wait((int *)0) < 0)
			fatal("Lost a subprocess");
}
/*
 * Read a message from the pipe and queue it up.
 */
rcvcmd(x)
register struct xfer *x;
{
	register struct transaction *t;
	register int n;
	struct pmesg pm;

	if ((n = read(x->x_pfd, (char *)&pm, sizeof(pm))) !=
	    sizeof(pm))
		fatal("Pipe read botch1: %d %d", n, errno);
	if ((t = salloc(transaction)) == TNULL)
		fatal(NOMEM);
	t->t_tid = savestr(pm.pm_tid);
	t->t_command = pm.pm_cmd;
	t->t_fh = x->x_fh;
	if (pm.pm_args) {
		if ((t->t_args = salloc(cmdargs)) == ANULL)
			fatal(NOMEM);
		ainit(t->t_args);
		t->t_args->a_num[0] = pm.pm_n;
	} else
		t->t_args = ANULL;
	syslog(LOG_INFO, "FILE: rcvd %s command\n", t->t_command->c_name);
	xqueue(t, x);
}
/*
 * Catch the close signal - cleanup!!
 */
closesig()
{
	signal(SIGHUP, SIG_IGN);
	longjmp(closejmp);
}
/*
 * The main routine of the subprocess
 */
dosubproc(x, t)
register struct xfer *x;
register struct transaction *t;
{
	int nread;

	if (x->x_options & O_DIRECTORY) {
		t->t_command = &dircom;
		xqueue(t, x);
	}
	/*
	 * If the close signal comes in, make the transfer done.
	 * The code in dowork will clean things up.
	 */
	if (setjmp(closejmp))
		x->x_state = X_DONE;
	else
		signal(SIGHUP, closesig);
	for (;;) {
		for (;;) {
			if (ioctl(x->x_pfd, FIONREAD, &nread) < 0)
				fatal("Failing FIONREAD");
			if (nread == 0)
				break;
			rcvcmd(x);
		}
		switch (dowork(x)) {
		case X_FLUSH:		/* Totally done */
			return;
		case X_CONTINUE:	/* In process */
			break;
		case X_HANG:		/* Need more instructions */
			rcvcmd(x);
		}
	}
	/* NOTREACHED */
}
#endif
