/* hostat --- read the host status of the specified host
 *
 * mbm@cipg -- works with 16bit and long status on 11 and vax
 *             fflush for each host
 *             tried to clean up output format
 * jek	    -- Improve output, allow interrupting out of each probe.
 */

#include <setjmp.h>
#include <sgtty.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include <hosttab.h>
#include <chaos.h>

int skip;
jmp_buf skipjmp;

/*
 * This KLUDGE is the only way to cleanly wait for the output to actually
 * be sent to the tty.
 */
void
odrain(int f)
{
#ifdef TIOCGETP
	struct sgttyb sg;

	ioctl(f, TIOCGETP, &sg);
	ioctl(f, TIOCSETP, &sg);
#endif
}

void
hostat(char *arg)
{
	char name[32];
	int f = -1;
	int addr;
	struct {
		short code;
		short nwords;
	} hdr;
	short unsigned s;
	long l;

	addr = chaos_addr(arg, 0);
	if (addr == 0) {
		printf("Host %s unknown\n",arg);
		return;
	}

	if (setjmp(skipjmp)) {
		if (f >= 0)
			close(f);
		printf(" Skipped\n");
		return;
	}

	printf("%s (0%o): ", arg, addr);
	fflush(stdout);
	skip++;		/* allow for skips */

	f = chopen(addr, "STATUS", 0, 0, 0, 0, 0);
	if (f < 0) {
		printf("not responding.\n");
		return;
	}

	read(f, name, sizeof(name));
	printf("%.32s\n", name);
	odrain(fileno(stdout));
	skip = 0;	/* Once he's seen the name, interrupts exit */

	printf("subnet rcvd       xmtd       abrt    lost");
	printf("    crc1    crc2    leng    rej\n");
	while (read(f, &hdr, sizeof(hdr)) == sizeof(hdr)) {
		if (hdr.code < 0400) {
			printf("%-7o", hdr.code);
			for (int i = 0; i < hdr.nwords; i++) {
				char *foo;

				foo = "%-8u";
				if (i < 2)
					foo = "%-11u";
				if (read(f, &s, sizeof(s)) >= 0)
					printf(foo, s);
				else
					fprintf(stderr, "read error\n");
			}
		} else if (hdr.code < 01000) {
			printf("%-7o", hdr.code & 0377);
			for (int i = 0; i < hdr.nwords; i += sizeof(long)/sizeof(short)) {
				char *foo = "%-8u";

				if (i < 2*sizeof(long)/sizeof(short))
					foo = "%-11u"; /* xtra for rcv,xmt*/
				if (read(f, &l, sizeof(l)) >= 0) {
					printf(foo, l);
				} else
					fprintf(stderr, "read error\n");
			}
		} else
			printf("unknown subnet code %o", hdr.code);
		putchar('\n');
	}
	close(f);
}

void
interrupt(int dummy)
{
	if (skip) {
		signal(SIGINT, interrupt);
		skip = 0;
		longjmp(skipjmp, 1);
	}
	exit(1);
}

int
main(int argc, char *argv[])
{
	struct host_entry *h;

	signal(SIGINT, interrupt);

	if (argc > 1) {
		while (--argc > 0)
			hostat(*++argv);
	} else {
		host_start();
		while ((h = host_next())) {
			if (chaos_addr(h->host_name, 0)) {
				printf("\n");
				fflush(stdout);
				hostat(h->host_name);
			}
		}
	}
	exit(0);
}
