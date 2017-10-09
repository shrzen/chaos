#include <stdio.h>
#include <ctype.h>
#include <errno.h>
#include <signal.h>
#include <sgtty.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/dir.h>
#include <sys/stat.h>
#ifndef pdp11
#include <wait.h>
#endif
#include <chaos/user.h>
#ifdef SYSLOG
#include <syslog.h>
#else
#define LOG_INFO 0
/* VARARGS 2 */
syslog(a,b,c,d,e,f,g,h)
char *b;
{
	fprintf(stderr, b,c,d,e,f,g,h);
}
#endif

int	dowrite(), dologin(), noserver();
char *word();
/* 
 * This is the unmatched RFC server.
 */
#define SERVERDIR	"/usr/local/lib/chaos"	/* For unknown servers */
#define DEV		"/dev"

#define LISTENER	0	/* There is another listener process, skip */
#define PROGRAM		1	/* There is a program to run */
#define FUNCTION	2	/* There is an internal function to execute */
#define C_TTY		01	/* Make channel into a tty */
#define C_IREAD		02	/* Allow reads from channel to return less
				   than requested (like tty) */
#define C_ACCEPT	04	/* Stupid server, must accept the RFC */
#define C_ANSWER	010	/* Server will send a datagram */

struct contact	{
	char	*c_name;	/* Contact name */
	char	c_flags;	/* Various flags for setting up the channel */
	char	c_type;		/* Server type - LISTENER, PROGRAM, FUNCTION*/
	char	*c_prog;	/* Program to run if c_type == PROGRAM */
	char	*c_arg0;	/* argv[0] for c_prog.
				   if null, use contact, if "", use prog */
	int	(*c_func)();	/* Function to call if FUNCTION */
	short	c_mode;		/* Mode to open the network channel with */
} random =
{	"",		C_ACCEPT,	PROGRAM,	"",			"",	0,	2,},
refused =
{	0,		0,		FUNCTION,	"",			0,	noserver,0,},
contacts[] = {
/*	name		flags		type		prog			arg0	func	mode*/
{	"SEND",		0,		PROGRAM,	"SEND",			0,	0,	0,},
{	"TELNET",	0,		PROGRAM,	"TELNET",			0,	0,	2,},
{	"UPTIME",	0,		PROGRAM,	"UPTIME",			0,	0,	1,},
{	"login",	0,		FUNCTION,	0,			0,	dologin,2,},
{	"who",		C_ACCEPT,	PROGRAM,	"/bin/who",		0,	0,	1,},
{	"w",		C_ACCEPT,	PROGRAM,	"/usr/ucb/w",		0,	0,	1,},
{	"finger",	C_ACCEPT,	PROGRAM,	"/usr/ucb/finger",	0,	0,	1,},
{	"pl",		0,		PROGRAM,	"/usr/local/pl",	0,	0,	1,},
{	"ftpread",	C_ACCEPT,	PROGRAM,	"/bin/cat",		"cat",	0,	1,},
{	"ftpwrite",	C_ACCEPT,	FUNCTION,	0,			0,	dowrite,0,},
{	"mail",		0,		PROGRAM,	"/etc/delivermail",	"",	0,	2,},
{	"write",	C_IREAD,	PROGRAM,	"/cs/src/cmd/chaos/chwrite",		0,	0,	0,},
{	0,		0,		0,		0,	0,	0,},
};

main()
{
	register int rfcfd;
	register char *cname;
	register struct contact *conp;
	char *args;
	extern int errno;
	char rfcbuf[CHMAXRFC + 1];
	int child();
	int i;

	if ((rfcfd = open(CHURFCDEV, 0)) < 0)
		prexit("%a: Cant open unmatched rfc handler: %s\n",
			CHURFCDEV);
	umask(0);
	chdir(SERVERDIR);
#ifndef pdp11
	sigset(SIGCHLD, child);
#endif
	initlogin();

	for (;;) {
		while ((i = read(rfcfd, rfcbuf, sizeof(rfcbuf))) < 0)
			if (errno != EINTR)
				prexit("%a: Ioctl, failed errno = %d\n", errno);
		rfcbuf[i] = '\0';
		syslog(LOG_INFO, "%a: Received RFC: '%s'\n", rfcbuf);
		cname = word(rfcbuf, &args);
		if (cname[0] == '\0') {
			syslog(LOG_INFO, "%a: empty rfc!\n");
			continue;
		}
		for (conp = contacts; conp->c_name != NULL; conp++)
			if (strcmp(cname, conp->c_name) == 0)
				break;
		if (conp->c_name == NULL) {
			if (access(cname, 1) == 0) {
				random.c_prog = cname;
				conp = &random;
			} else
				conp = &refused;
			conp->c_name = cname;
		}
		docontact(conp, args, rfcfd);
	}
}
/*
 * Do the appropriate thing with the contact that has been found
 * acceptable in the table.
 */
docontact(conp, args, rfcfd)
register struct contact *conp;
char *args;
int rfcfd;
{
	int chfd;
	char *argv[CHMAXARGS];
	char listenname[sizeof(CHLISTDEV) + 1 + CHMAXDATA];

	/*
	 * If there should be another permanent listener process,
	 * just skip it.
	 */
	if (conp->c_type == LISTENER) {
		ioctl(rfcfd, CHIOCRSKIP, 0);
		syslog(LOG_INFO, "%a: skipped RFC: '%s %s'\n",
			conp->c_name, args);
		return;
	}
	/*
	 * Open the connection, taking the RFC off the unmatched RFC list.
	 * If it fails, it either timeout aleady or was grabbed by another
	 * listener, both of which are unlikely.
	 */
	sprintf(listenname, "%s/%s", CHLISTDEV, conp->c_name);
	if ((chfd = open(listenname, conp->c_mode)) < 0)
		syslog(LOG_INFO, "%a: listen failed on RFC: '%s %s'\n",
			conp->c_name, args);
	/*
	 * Fork to do the real work, closing appropriate files.
	 */
	else switch (fork()) {
	default:
#ifdef pdp11
		wait(0);
#endif
		close(chfd);
		break;
	case -1:
		syslog(LOG_INFO, "%a: fork failed on '%s %s'\n",
			conp->c_name, args);
		ioctl(chfd, CHIOCREJECT, "No process available for server.");
		break;
	case 0:
#ifdef pdp11
		if (fork())
			exit();
#endif
		if (conp->c_flags & C_TTY)
			ioctl(chfd, CHIOCSMODE, CHTTY);
		close(rfcfd);
		close(0);
		close(1);
		close(2);
		if (conp->c_type == PROGRAM) {
			/*
			 * Here we execute a server program.  Note that the
			 * server must know enough about the Chaosnet to
			 * either CHIOCACCEPT, CHIOCANSWER, or CHIOCREJECT
			 * the connection. Unless the C_ACCEPT is on for the
			 * table entry under the contact name.
			 */
			if (makeargv(args, argv + 1)) {
				ioctl(chfd, CHIOCREJECT,
					"Too many words in RFC.");
				syslog(LOG_INFO,
					"%a: too many args in RFC: '%s %s'\n",
					conp->c_name, args);
				exit(1);
			}
			if (conp->c_arg0 == NULL)
				argv[0] = conp->c_name;
			else if (*conp->c_arg0 == '\0')
				argv[0] = conp->c_prog;
			else
				argv[0] = conp->c_arg0;
			if ((conp->c_mode & 1) == 0)
				dup2(chfd, 0);
			else
				open("/dev/null", 0);
			if (conp->c_mode > 0)
				dup2(chfd, 1);
			else
				open("/dev/null", 2);
			dup2(1, 2);
			if (conp->c_flags & C_ACCEPT)
				ioctl(chfd, CHIOCACCEPT, 0);
			else if (conp->c_flags & C_ANSWER)
				ioctl(chfd, CHIOCANSWER, 0);
			close(chfd);
			execv(conp->c_prog, argv);
			syslog(LOG_INFO, "%a: exec failed on %s",
				conp->c_prog);
			if ((conp->c_flags & C_ACCEPT) == 0)
				ioctl(chfd, CHIOCREJECT,
					"Server program doesn't exist");
		} else
			(*conp->c_func)(conp->c_name, args, chfd);
		exit(1);
	}
}
/*
 * File writing server - NO PROTECTION CHECKING!! for testing only.
 *
 */
dowrite(cname, args, chfd)
char *cname;
char *args;
int chfd;
{
	register int fd;

	dup2(chfd, 0);
	close(chfd);
	if (creat(word(args, 0), 0666) >= 0) {
		open("/dev/null", 2);
		execl("/bin/cat", "cat", NULL);
	}
	syslog(LOG_INFO, "%a: ftpwrite create failed: %s\n", args);
}
/*
 * Login initialization - just get the major device to use for net tty's
 */
int chanmaj;
initlogin()
{
	struct stat sbuf;
	if (stat(CHCPRODEV, &sbuf) < 0) {
		syslog(LOG_INFO, "%a: no chaos prototype: %s, exiting",
			CHCPRODEV);
		prexit("%a: no chaos prototype: %s, exiting",
			CHCPRODEV);
	}
	chanmaj = major(sbuf.st_rdev);
}
/*
 * Login server.
 */
dologin(cname, args, chfd)
char *cname;
char *args;
int chfd;
{
	register int i;
	register int fd;
	register FILE *fp;
	char *termcap;
	struct chstatus chst;
	char ttyname[DIRSIZ + 1];
	char devname[sizeof(DEV) + 1 + DIRSIZ];
	char tabname[20 + 1 + DIRSIZ];

	/*
	 * Get the status of the connection, mainly for the local channel
	 * number and remote host.
	 */
	if (ioctl(chfd, CHIOCGSTAT, &chst) < 0) {
		syslog(LOG_INFO, "%a: can't read RFC for: login '%s'\n",
			args);
		ioctl(chfd, CHIOCREJECT, "Server program error.");
		return;
	}
	/*
	 * Make a ttyname, mapping channel number to 0-9a-zA-Z.
	 */
	sprintf(ttyname, "ttyC%c", chst.st_cnum + (
		chst.st_cnum <= 9 ? '0' :
		chst.st_cnum <= 9 + 'z' - 'a' + 1 ? 'a' - 10 :
		'A' - 10 - ('z' - 'a' + 1)));
	sprintf(devname, "/dev/%s", ttyname);
	sprintf(tabname, "%s/%s", "foobar", ttyname);
	/*
	 * If the /dev/ttyC? entry exists it is probably either around from
	 * a crash or /etc/init hasn't gotten around to flushing it.
	 * Delay to give /etc/init a chance, and then flush it (as well as the
	 * /etc/ttytab entry) in any case.
	 */
	if (access(devname, 0) == 0)
		sleep(5);
	unlink(devname);
	unlink(tabname);
	/*
	 * Make the /dev entry, using the channel number as the
	 * minor device number, and the major device number from the
	 * chaos channel prototype device.
	 */
	if (mknod(devname, S_IFCHR | 0622, makedev(chanmaj, chst.st_cnum)) < 0) {
		ioctl(chfd, CHIOCREJECT, "System Error: Can't make new tty.");
		syslog(LOG_INFO, "%a: can't make node: %s\n", devname);
	/*
	 * Create the TTYTAB file describing the tty, taking entries
	 * from the RFC string.
	 */
	} else if ((fp = fopen(tabname, "w")) == NULL) {
		ioctl(chfd, CHIOCREJECT, "System Error: Can't setup tty.");
		syslog(LOG_INFO, "%a: can't create tty info file: %s\n",
			tabname);
	} else {
		if ((termcap = word(args, &args)) != NULL)
			fprintf(fp, "t %s\n", termcap);
		if (*args)
			fprintf(fp, "l %s\n", args);
		fprintf(fp, "p net\n");
		fprintf(fp, "i n\n");
		fflush(fp);
		if (ferror(fp)) {
			unlink(tabname);
			ioctl(chfd, CHIOCREJECT,
				"System Error: Can't setup tty.");
			syslog(LOG_INFO, "%a: error writing %s", tabname);
		/*
		 * Turn the chaos channel device into a tty (CHIOCTTY).
		 * This only work using a file descriptor from opening the
		 * channel device, NOT the original file descriptor from the
		 * listen since it does not represent an individual channel.
		 */
		} else if ((fd = open(devname, 2)) < 0 ||
			   ioctl(fd, CHIOCSMODE, CHTTY) < 0) {
			unlink(devname);
			unlink(tabname);
			ioctl(chfd, CHIOCREJECT,
				"System Error: Can't open network tty.");
			syslog(LOG_INFO, "%a: open of %s failed", devname);
		} else {
			fclose(fp);
			/*
			 * Change the mode of the TTYTAB file to indicate that
			 * a login should be started (the user exec (0100))
			 * and that it is a network tty that should be flushed
			 * when the login (shell etc) process terminates (the
			 * group exec (010)).
			 */
			chmod (tabname, 0754);
			syslog(LOG_INFO, "%a: chaos login on %s (%s) from host %x",
				devname, tabname, chst.st_fhost);
			ioctl(chfd, CHIOCACCEPT, 0);
			close(chfd);
			/*
			 * Now we signal /etc/init to scan the TTYTAB, at
			 * which time it should find the entry we just created
			 * and enabled.  We must sleep here to make sure that
			 * someone has the channel open between now and when
			 * /etc/init opens the /dev entry to start up a login
			 * process. If /etc/init takes too long to do this,
			 * the channel is just closed, the other end gets a
			 * hangup, and init eventually gets an error when it
			 * tries to open it too late.
			 */
			kill(1, SIGHUP);
			sleep(60);
		}
	}
	/* forked process exits here, closing everything */
}
/*
 * Refuse the connection - there is no server.
 */
noserver(cname, args, chfd)
char *cname;
char *args;
int chfd;
{
	syslog(LOG_INFO, "%a: contact '%s %s' refused.\n", cname, args);
	ioctl(chfd, CHIOCREJECT, "No server for contact name");
}
#ifndef pdp11
/*
 * Handle child interrupts - just gobble the status to flush the zombie
 */
child() {
	union wait w;
	int pid;

	while( (pid = wait3(&w.w_status, WNOHANG, 0)) > 0) ;
}
#endif
/*
 * Break up the given string into words, filling a pointer array.
 * Returns 0 if ok, !0 if more words than CHMAXARGS.
 */
makeargv(string, argp)
char *string;
register char **argp;
{

	char *cp = string;
	register int nargs = 0;

	while (*cp)
		if (++nargs > CHMAXARGS) {
			syslog(LOG_INFO, "%a: Too many words in RFC: '%s'\n",
				string);
			return(1);
			break;
		} else
			*argp++ = word(cp, &cp);
	*argp++ = NULL;
	return 0;
}
/*
 * Find a word, return a pointer to it, null terminate it, and update
 * *nextcp, to be able to use it to find the next word.
 */
char *
word(cp, nextcp)
register char *cp;
char **nextcp;
{
	register char *startp;

	while (isspace(*cp))
		cp++;
	startp = cp;
	if (*cp != '\0') {
		while (*++cp != '\0')
			if (isspace(*cp))
				break;
		if (*cp != '\0')
			*cp++ = '\0';	/* Null terminate that string */
	}
	if (nextcp)
		*nextcp = cp;
	return (startp);
}
prexit(s, a, b, c, d)
char *s;
{
	fprintf(stderr, a, b, c, d);
	exit(1);
}
