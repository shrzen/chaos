/*
 * Chaosnet RFC handler for contact name MAIL
 */

#include <stdio.h>
#include <signal.h>
#include <chaos/user.h>
#include <pwd.h>
#include <wait.h>

#define LINESIZE 132
#define MAXLIST 100
char buf[BUFSIZ];
char line[LINESIZE];
char *tolist[MAXLIST];

main(argc, argv)
int argc;
char *argv[];
{
	register int c;
	register FILE *to;
	register char *p, **t;
	int pid, cnt;
	char *error;
	struct passwd *getpwnam();
	struct passwd *pwd;
	int nnl;		/* number of newline characters */
	int pip[2];
	extern int errno;
	union wait w;
	char *malloc();


	setbuf(stdout, buf);	/* "You never know, with Standard I/O" */
	/*
	 * Do all the things that might cause us errors so we can punt right
	 * away and avoid hairy error processing.
	 */
	if ((pwd = getpwnam("network")) == NULL)
		error = "-Can't find uid for user: network";
	else
		setuid(pwd->pw_uid);
	t = tolist;
	error = NULL;
	cnt = 0;
	*t++ = "-chaosmail";
	*t++ = "-ee";	/* return mail and don't complain */
	*t++ = "-i";	/* don't look for a period at BOL for EOF */
	*t++ = "-a";	/* Find sender by parsing header */
	*t++ = "-m";	/* Include sender in aliasing */

	for (;;) {
		for (p = line; (c = getchar()) != CHNL; p++)
			if (c == EOF) {
				ioctl(fileno(stdin), CHIOCREJECT,
					"Protocol error - Unexpected EOF");
				exit(1);
			} else if (p < &line[LINESIZE])
				*p = c;
		if (p == line)
			break;
		else if (p >= &line[LINESIZE])
			printf("-Recipient name too long%c", CHNL);
		else {
			*p = 0;
			while (p > line && *--p != '@')
				;
			if (p > line && *p == '@' &&
			    strcmp(host_me(), host_name(p + 1)) == 0)
				*p = '\0';
			p++;
			if (t >= &tolist[MAXLIST - 1])
				printf("%%Too many recipients%c", CHNL);
			else if ((*t = malloc(strlen(line) + 1)) == NULL)
				printf("%%Not enough memory for recipients%c",
					CHNL);
			else {
				strcpy(*t++, line);
				printf("+%c", CHNL);
				cnt++;
			}
		}
		fflush(stdout);
		ioctl(fileno(stdout), CHIOCFLUSH, 0);
	}
	if (cnt == 0)
		error = "-No recipients for this message";
	*t = 0;
	if (error == 0 && pipe(pip) < 0)
		error = "%Can't open pipe to delivermail";
	if (error == 0)
		switch(pid = fork()) {
		case 0:
			close(0);
			dup(pip[0]);
			close(1);
			close(pip[0]);
			close(pip[1]);
			execv("/etc/delivermail", tolist);
			exit(errno);
		case -1:
			error = "%Can't fork delivermail";
			break;
		}
	if (error) {
		while (getchar() != EOF)
			;	/* swallow input */
		printf("%s%c", error, CHNL);
	} else {
		close(pip[0]);
		to = fdopen(pip[1], "w");
		nnl = 0;
		while ((c = getchar()) != EOF) {
			switch(c) {
			case 0210:
			case 0211:
			case 0214:
				c -= 0200;
				break;

			case 0215:
				c = '\n';
				break;
			}
			if (c == '\n')
				nnl++;
			else
				nnl = 0;
			putc(c, to);
		}
		/*
		 * Delivermail always adds a blank line itself.
		 * But we make sure at least, that there is a whole line.
		 */
		if (nnl == 0)
			putc('\n', to);
		if (ferror(stdin) || ferror(to)) {
			kill(pid, SIGTERM);
			printf("%%I/O error in copying to delivermail%c",
				CHNL);
			/*
			 * We don't bother waiting for delivermail.
			 * /etc/init will get it anyway.
			 */
		} else {
			/*
			 * When we close, delivermail will do its thing.
			 */
			fclose(to);
			wait(&w);	/* This just can't lose */
			if (w.w_status == 0)
				printf("+%c", CHNL);
			else
				printf("%%delivermail failed: sig:%d dump:%d code:%d%c",
				w.w_termsig, w.w_coredump, w.w_retcode, CHNL);
		}
	}
	fflush(stdout);	/* Just for safety */
	ioctl(fileno(stdout), CHIOCFLUSH, 0);
	return(0);
}
