// signal.c --- UNIX signal code

#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/un.h>

#include <netinet/in.h>

#include "log.h"

int got_sighup;
int got_sigterm;
int got_sigchld;

void
sighup_handler(int sig)
{
	got_sighup++;
}

void
sigterm_handler(int sig)
{
	got_sigterm++;
}

void
sigpipe_handler(int sig)
{
	printf("sigpipe!\n");
}

void
sigchld_handler(int sig)
{
	printf("sigchld!\n");
	got_sigchld++;
}

#define SIGACTION(sig, handler)				\
	do {						\
		memset((char *) &new, 0, sizeof(new));	\
		new.sa_handler = handler;		\
		sigaction(sig, &new, &old);		\
	} while (0)

int
signal_init(void)
{
	struct sigaction new, old;
	
	SIGACTION(SIGTERM, sigterm_handler);
	SIGACTION(SIGPIPE, sigpipe_handler);
	SIGACTION(SIGCHLD, sigchld_handler);
	
	return 0;
}

void
signal_poll(void)
{
	if (got_sighup) {
		debugf(DBG_WARN, "got signal SIGHUP; ignoring\n");
	}
	
	if (got_sigterm) {
		debugf(DBG_WARN, "got signal SIGTERM; shutting down\n");
		server_shutdown();
	}
	
	if (got_sigchld) {
		debugf(DBG_WARN, "got signal SIGCHLD; child died\n");
		restart_child();
	}
}
