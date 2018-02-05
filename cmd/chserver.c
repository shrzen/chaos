/* chserver --- chaos server dispatcher program
 *
 * It processes the unmatched RFC queue and starts programs running
 * which it finds in DESTSERVERS and whose file names match the
 * contact string in the RFC.
 *
 * 8/15/84 Cory Myers
 *	Change parsing in word to use CHLF if input is from a LISPM
 *
 * 8/17/84 dove
 *	adjust syslog priorities
 *	openlog(argv[0], 0)
 *	get hostname on bad rfc's
 *
 * Cory Myers 9/24/84
 *	server name can not containing '/'
 *
 * Revision 1.3  85/02/27  11:06:46  dove
 * Correct bug in rejecting servers that can't be exec'd
 *
 * Revision 1.2  84/12/13  19:39:15  dove
 * Don't syslog refusals of WHO-AM-I
 *
 * Revision 1.1  84/12/13  19:26:11  dove
 * Initial revision
 *
 */

#include <ctype.h>
#include <errno.h>
#include <fcntl.h>
#include <sgtty.h>
#include <signal.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <syslog.h>
#include <unistd.h>
#include <wait.h>

#include <sys/dir.h>
#include <sys/ioctl.h>
#include <sys/param.h>
#include <sys/stat.h>
#include <sys/stat.h>
#include <sys/types.h>

#include <hosttab.h>
#include <chaos.h>

int is_lispm;
int debug = 1;

/*
 * This is the unmatched RFC server.
 */
#ifndef DESTSERVERS
#define DESTSERVERS	"/usr/chaos/libexec"	/* For unknown servers */
#endif

/*
 * Find a word, return a pointer to it, null terminate it, and update
 * *nextcp, to be able to use it to find the next word.
 */
#define space(x)							\
	((!is_lispm && ((x) == ' ' || (x) == '\t')) || (x) == CHLF)

char *
word( char *cp,  char **nextcp)
{
	char *startp;

	while (space(*cp))
		cp++;
	startp = cp;
	if (*cp != '\0') {
		while (*++cp != '\0')
			if (space(*cp)) break;
		if (*cp != '\0')
			*cp++ = '\0';	/* Null terminate that string */
	}
	if (nextcp)
		*nextcp = cp;
	return startp;
}

/*
 * Break up the given string into words, filling a pointer array.
 * Returns 0 if ok, !0 if more words than CHMAXARGS.
 */
int
makeargv(char *string,  char **argp)
{

	char *cp = string;
	int nargs = 0;

	while (*cp) {
		if (++nargs > CHMAXARGS)
			return 1;
		*argp++ = word(cp, &cp);
	}
	*argp++ = NULL;
	return 0;
}

void
refuse(char *cname, char *error)
{
	int chfd;
	char clserror[CHMAXDATA];
	struct chstatus chst;

	chfd = chlisten(cname, 0, 1, 0);
	if (chfd < 0) {
		syslog(LOG_NOTICE, "cannot 'listen' on %s\n",
		       cname);
		return;
	}

	if (chstatus(chfd, &chst) < 0)
		syslog(LOG_NOTICE, "Bad STAT on rejected RFC");
	sprintf(clserror, "%s for contact name: %s", error, cname);
	if (strcmp(cname, "WHO-AM-I")!=0)
		syslog(LOG_NOTICE, "Host %s rejected: %s\n",
		       chaos_name(chst.st_fhost), clserror);
	chreject(chfd, clserror);
	close(chfd);
}

/*
 * Do the appropriate thing with the contact that has been found
 * acceptable in the table.
 */
void
docontact(char *cname,  char *args)
{
	int chfd;
	int mode;
	char *argv[CHMAXARGS];
	struct chstatus chst;
	struct stat sbuf;

	/*
	 * Cory Myers 9/24/84
	 *	server name can not contain '/'
	 */
	if (index(cname,'/') != NULL) {
		refuse(cname, "Illegal server name");
		return;
	}

	if (stat(cname, &sbuf) < 0) {
		refuse(cname, "No server exists");
		return;
	} else if ((sbuf.st_mode & 0111) == 0) {
		refuse(cname, "Server not executable");
		return;
	}

	/*
	 * Group permissions used to indicate open modes.
	 */
	mode = (sbuf.st_mode & 020 ?
		(sbuf.st_mode & 040 ? 2 : 1) : 0);

	/*
	 * Open the connection, taking the RFC off the unmatched RFC list.
	 * If it fails, it either timeout aleady or was grabbed by another
	 * listener, both of which are unlikely.
	 */
	if ((chfd = chlisten(cname, mode, 1, 0)) < 0) {
		syslog(LOG_NOTICE, "listen failed on RFC: '%s %s'\n",
		       cname, args);
		return;
	}

	if (chstatus(chfd, &chst) < 0 ||
	    chst.st_state != CSRFCRCVD) {
		syslog(LOG_NOTICE,
		       "bad listen status for: '%s %s'\n",
		       cname, args);
		close(chfd);
		return;
	}

	/* gdt 11/5/88 - log hostname too! */
	syslog(LOG_INFO, "RFC %s(%s) from %s\n",
	       cname, args, chaos_name(chst.st_fhost));

	/*
	 * Fork to do the real work, closing appropriate files.
	 */
	switch (fork()) {
	default:
		close(chfd);
		break;
	case -1:
		syslog(LOG_NOTICE, "fork failed on '%s %s'\n",
		       cname, args);
		chreject(chfd, "No process available for server.");
		close(chfd);
		break;
	case 0:
		for (int i = 0; i < NOFILE; i++)
			if (i != chfd)
				close(i);
		if (makeargv(args, argv + 1)) {
			chreject(chfd, "Too many words in RFC.");
			syslog(LOG_NOTICE,
			       "too many args: '%s %s'\n",
			       cname, args);
			exit(1);
		}
		argv[0] = cname;
		dup2(chfd, 0);
		dup2(chfd, 1);
		dup2(1, 2);
		close(chfd);
		execv(cname, argv);
		chreject(1, "Can't EXEC server");
		exit(1);
	}
}

/*
 * Handle child interrupts - just gobble the status to flush the zombie
 */
void
child(int dummy)
{
	int w;
	pid_t pid;

	while ((pid = wait3(&w, WNOHANG, 0)) > 0);
}

int
main(void)
{
	int rfcfd;
	char *cname;
	char *args;
	extern int errno;
	char rfcbuf[CHMAXRFC + 1];
	int i;
	int tt;

	openlog("chserver", 0, LOG_USER);

	rfcfd = open(CHURFCDEV, 0);
	if (rfcfd < 0) {
		syslog(LOG_ERR,
		       "Can't open unmatched rfc handler: %s\n",
		       CHURFCDEV);
		exit(1);
	}

	umask(0);
	chdir(DESTSERVERS);
	signal(SIGCHLD, child);

	/* Flush control tty and run in background.  */
	tt=open("/dev/tty", 0);
	if (tt) {
		ioctl(tt, TIOCNOTTY,0);
		close(tt);
	}

	if (!debug && fork())
		exit(0);

	for (;;) {
		while ((i = read(rfcfd, rfcbuf, sizeof(rfcbuf))) < 0) {
			if (errno != EINTR) {
				syslog(LOG_ERR,
				       "read failed errno:%d,i:%d\n",
				       errno, i);
				exit(1);
			}
		}
		rfcbuf[i] = '\0';

		/* gdt - logging moved to docontact */
		is_lispm = 0;
		cname = word(rfcbuf, &args);
		is_lispm = strchr(args,CHLF) == NULL ? 0 : 1;
		if (cname[0] == '\0')
			syslog(LOG_NOTICE, "empty rfc!\n");
		docontact(cname, args);
	}
}
