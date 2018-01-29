#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <time.h>
#include <sys/ioctl.h>
#include <sys/file.h>
#include <sys/stat.h>
#include <signal.h>
#include <sgtty.h>
#include <errno.h>
#include <hosttab.h>

/*
 *	CONNECT server
 */

#include <chaos.h>

#define SCPYN(a, b)	strncpy(a, b, sizeof(a))
#define SCMPN(a, b)	strncmp(a, b, sizeof(a))
#define BANNER		"\r\n\r\n4.3 BSD UNIX (%s)\r\n\r\r\n\r%s"

char chtty[] = "/dev/tty\0\0\0";
extern char **environ;

char remote_hostname[40],local_hostname[40];

int onchild();
#define NJOBS 128
struct job{ int pid; } jobs[NJOBS];
main(argc, argv)
char *argv[];
{

#define err stderr

	int chfd = -1;
	int f;
	f = open("/dev/tty", O_RDWR);
	ioctl(f, TIOCNOTTY);
	close(f);
	close(0); close(1); close(2);
	freopen("/usr/tmp/connect", "w", stderr);	
	setbuf(stderr, NULL);
	signal(SIGCHLD, onchild);
	signal(SIGHUP, SIG_IGN);

	for (;;){
	  /* listen for CONNECT contacts */
	  int signalmask;
	  close(chfd);
	  chfd = chlisten("CONNECT", 2, 1, 0);
/*	  signalmask = sigblock(sigmask(SIGCHLD));*/
	  if (chfd < 0)
	    perror("chopen"),fflush(err);
	  else 
	  {
		if (chwaitfornotstate(chfd, CSLISTEN) == 0)
		  connect(chfd);
	  }
	  fflush(stderr);
/*	  sigsetmask(signalmask);*/
	}
}
connect(fd)
{
	struct chstatus chstat;
  	int pid;
	register char* p;
	int unit;
	int f;

	
	chstatus(fd, &chstat);
	if (chstat.st_state != CSRFCRCVD)
	  { fprintf(err, "? conn state %d\n", chstat.st_state);
	    return(-1);
	  }
	gethostname(local_hostname,sizeof(local_hostname));
	p = chaos_name(chstat.st_fhost);
	if (p)
		SCPYN(remote_hostname,p);
	else
		sprintf(remote_hostname, "chaos %o",chstat.st_fhost);
	fprintf(err,remote_hostname);
	if ((unit = open("/dev/tty", O_RDWR|O_NDELAY)) != -1)
	{
	  ioctl(unit, TIOCNOTTY);
	  close(unit);
	}
	unit = -1;

	if (ioctl(fd, CHIOCGTTY, &unit) )
	{
	  chreject(fd, "All chttys in use");
	  perror("chiocgtty");
	  fflush(err);
	  return(-1);
	}
	else
		fprintf(err," unit %x\n", unit);
/*	chtty[strlen(chtty) - 1] = unit + '0';*/
/*	sprintf(&chtty[strlen("/dev/tty")],"%c%c",(unit/10)+'A',(unit%10)+'0');*/
	sprintf(&chtty[strlen("/dev/tty")],"%02x",unit);
	chtty[strlen("/dev/tty")] += 'v'-'0' ;

	fflush(err);
	
	pid = fork();
	if (pid){
	  /* associate pid with chtty */
	  jobs[unit].pid = pid;
	  close (fd);
	  return 0;
	}

	/* child */

	unit = open(chtty,O_RDWR);
/*	setpgrp(0, getpid());*/
	if (unit < 0)
	{
	  chreject(fd, "Cant open chtty");
	  exit(1);
	}

	close(fd);
	getty(chtty,unit);
}
/* stolen from Web Dove supdup server */
getty(tty,ftty)
{
    short mw;
    int f;

#ifdef notdef
    if ((f = open("/dev/tty",O_RDWR)) >= 0) { /* flush my cntrl tty */
      ioctl(f,TIOCNOTTY,0); close(f);
    };
    if ((f = open(tty, O_RDWR | O_NDELAY)) < 0) exit(1);
    close(ftty);    
    signal(SIGHUP,SIG_IGN); 
/*    vhangup();*/
    close(f);
/*    sleep(1);*/
    signal(SIGHUP,SIG_DFL);
    if ((f = open("/dev/tty",O_RDWR)) >= 0) {	/* flush my cntrl tty */
        ioctl(f,TIOCNOTTY,0); close(f);
    };
    if ((ftty = open(tty, O_RDWR | O_NDELAY)) < 0) exit(1);
#endif
    errno = 0;
    chown(tty,0,0); chmod(tty,0622);	/* make the tty reasonable */
    ioctl(ftty,TIOCNXCL,0);		/* turn off excl use */
    dup2(ftty,0); close(ftty); dup2(0,1); dup2(0,2);	/* Connect to tty */
    printf(BANNER,local_hostname,"");
    execl("/bin/login","login","-h",remote_hostname,0);
    perror("failed to exec login!\n\r");
    fflush(err);
    exit(1);
}

#include <sys/wait.h>
onchild(sig)
{
  int child;
  int i;
  fprintf(err, "signal %d\n", sig);
 loop:
  child =  wait3(0,WNOHANG,0);
  fprintf(err, "wait returned %D\n", child);
  fflush(err);
  if (child <= 0) return;
  for (i = 0; i < NJOBS; i++)
    if (jobs[i].pid == child)
    {
      char *line = "ttyxx";
      char *dev = "/dev/ttyxx";
      int f;
/*      sprintf(line,"tty%c%c",(i/10)+'A',(i%10)+'0');*/
	sprintf(line,"tty%02x",i);
        line[strlen("tty")] += 'v'-'0' ;

      rmut(line);
      fprintf(err, "rmut %s\n", line);

/* driver must support O_NDELAY open for this to work */
/*      sprintf(dev,"/dev/tty%c%c",(i/10)+'A',(i%10)+'0');*/
	sprintf(dev,"/dev/tty%02x",i);
        dev[strlen("/dev/tty")] += 'v'-'0' ;


      if ((f = open("/dev/tty", O_RDWR|O_NDELAY)) >= 0)
	  { ioctl(f, TIOCNOTTY); close(f); }

      if ((f = open(dev, O_RDWR|O_NDELAY)) >= 0)
      {
	    int f1;
/*	    vhangup();*/
	    if ((f1 = open("/dev/tty", O_RDWR|O_NDELAY)) >= 0)
		{ ioctl(f1, TIOCNOTTY); close(f1); }
	    sleep(1);
	    close(f);
      }
/* */
    }
  goto loop;
}

ontimer(){}


#include <utmp.h>
rmut(line)
    char *line;
{
    struct utmp wtmp;
    register int f;
    int found = 0;

    f = open("/etc/utmp", O_RDWR);
    if (f >= 0) {
        while (read(f, (char *)&wtmp, sizeof(wtmp)) == sizeof(wtmp)) {
            if (SCMPN(wtmp.ut_line, line) || wtmp.ut_name[0]==0)
                continue;
            lseek(f, -(long)sizeof(wtmp), 1);
            SCPYN(wtmp.ut_name, "");
            SCPYN(wtmp.ut_host, "");
            time(&wtmp.ut_time);
            write(f, (char *)&wtmp, sizeof(wtmp));
            found++;
        }
        close(f);
    }
    if (found) {
        f = open("/usr/adm/wtmp", O_WRONLY|O_APPEND);
        if (f >= 0) {
            SCPYN(wtmp.ut_line, line);
            SCPYN(wtmp.ut_name, "");
            SCPYN(wtmp.ut_host, "");
            time(&wtmp.ut_time);
            write(f, (char *)&wtmp, sizeof(wtmp));
            close(f);
        }
    }
}
